#include <iostream>
#include <string>
#include <sys/inotify.h>
#include <unistd.h>
#include <vector>

#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <unordered_map>

#include <cstring>

#include <fnmatch.h>
#include <mutex>
#include <thread>

#include "error/error.h"
#include "error/result.h"
#include "include/filewatcher.h"
#include "include/macros.hpp"

using namespace std;
namespace kilket {

Result<FileWatcher *> FileWatcher::create() {
  FileWatcher *watcher = new FileWatcher();
  TEST_OVERLOADED(watcher->init(), FileWatcher *);
  return Result<FileWatcher *>::Ok(watcher);
}

Result<void> FileWatcher::init() {
  FW_LOG("[DEBUG] Initializing FileWatcher...");
  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd == -1) {
    return Result<void>::Err(FWError::make(ErrorCode::SYS_IO_FAILED,
                                           "Error: inotify_init1 failure. ✗"));
  }
  poll_target_count = 1;
  poll_targets[0].fd = inotify_fd;
  poll_targets[0].events = POLLIN;
  FW_LOG("[DEBUG] FileWatcher initialized successfully. ✓");
  return Result<void>::Ok();
}

// Reads whatever is sitting in the inotify fd's buffer and translates the
// first raw inotify_event it finds into our own WatchEvent shape.
Result<WatchEvent> FileWatcher::read_next_event(int fd, vector<int> watched_descriptors,
                                                int descriptor_count) {
  WatchEvent result;
  const struct inotify_event *raw_event;
  char buffer[4096];
  ssize_t bytes_read;

  for (;;) {
    bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read == -1 && errno != EAGAIN) {
      std::string message =
          "Error: read failure; errno: " + std::string(strerror(errno));
      return Result<WatchEvent>::Err(
          FWError::make(ErrorCode::SYS_IO_FAILED, message));
    }

    if (bytes_read <= 0)
      break;

    int seen = 0;
    for (char *cursor = buffer; cursor < buffer + bytes_read;) {
      raw_event = reinterpret_cast<const struct inotify_event *>(cursor);

      if (raw_event->mask & IN_CLOSE_WRITE) {
        FW_LOG("[DEBUG] - EVENT detected - IN_CLOSE_WRITE");
        result.event_mask = IN_CLOSE_WRITE;
      } else if (raw_event->mask & IN_MODIFY) {
        FW_LOG("[DEBUG] - EVENT detected - IN_MODIFY");
        result.event_mask = IN_MODIFY;
      } else if (raw_event->mask & IN_MOVED_TO) {
        FW_LOG("[DEBUG] - EVENT detected - IN_MOVED_TO");
        result.event_mask = IN_MOVED_TO;
      } else if (raw_event->mask & IN_MOVED_FROM) {
        FW_LOG("[DEBUG] - EVENT detected - IN_MOVED_FROM");
        result.event_mask = IN_MOVED_FROM;
      } else {
        FW_LOG("[DEBUG] - EVENT detected - EVENT_MASK: " << raw_event->mask);
      }

      string base_path = watch_registry[raw_event->wd];
      if (raw_event->len > 0) {
        result.path = base_path + "/" + raw_event->name;
      } else {
        result.path = base_path;
      }
      result.wd = raw_event->wd;
      result.filetype = (raw_event->mask & IN_ISDIR) ? "dir" : "file";

      cursor += sizeof(struct inotify_event) + raw_event->len;
      if (seen <= descriptor_count)
        seen++;
      return Result<WatchEvent>::Ok(result);
    }
  }
  return Result<WatchEvent>::Err(ErrorCode::EVENT_NOT_FOUND,
                                 "Error: empty event. ✗");
}

Result<void> FileWatcher::add_path(const string &arg) {
  lock_guard<mutex> lock(registry_mutex);
  for (auto [wd, path] : watch_registry) {
    if (path == arg) {
      return Result<void>::Err(
          ErrorCode::PATH_ALREADY_EXISTS,
          "Error: path " + arg + " already exists ✗");
    }
  }
  int wd = inotify_add_watch(inotify_fd, arg.c_str(),
                             IN_MOVED_TO | IN_MOVED_FROM | IN_MODIFY |
                                 IN_CLOSE_WRITE);
  if (wd == -1)
    return Result<void>::Err(FWError::make(
        ErrorCode::SYS_IO_FAILED, "Error: inotify_add_watch failure on path " + arg + " ✗"));
  watch_registry[wd] = arg;
  reverse_watch_registry[arg] = wd;
  FW_LOG("[DEBUG] Adding path " + arg +
         " to filewatcher completed. ✓");
  return Result<void>::Ok();
}

Result<void> FileWatcher::remove_path(const string &arg) {
  lock_guard<mutex> lock(registry_mutex);
  bool path_exists = false;
  for (auto [wd, path] : watch_registry) {
    if (arg == path) {
      path_exists = true;
    }
  }
  if (!path_exists) {
    return Result<void>::Err(
        FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: Path not found"));
  }

  int wd = reverse_watch_registry[arg];
  inotify_rm_watch(inotify_fd, wd);
  watch_registry.erase(wd);
  reverse_watch_registry.erase(arg);
  return Result<void>::Ok();
}

Result<void> FileWatcher::link_event(uint32_t event_mask,
                                     WatchCallback callback) {
  lock_guard<mutex> lock(registry_mutex);

  FW_LOG("[DEBUG] Linking event: event_mask = " + to_string(event_mask) +
         " to callback ...");
  if (event_mask != IN_MODIFY && event_mask != IN_CLOSE_WRITE &&
      event_mask != IN_MOVED_TO && event_mask != IN_MOVED_FROM) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EVENT_NOT_SUPPORTED,
                      "Error: no such event " + to_string(event_mask) + ". ✗"));
  }

  for (auto &existing : event_callbacks[event_mask]) {
    if (existing == callback) {
      return Result<void>::Err(
          FWError::make(ErrorCode::DUPLICATE_ENTRY,
                        "Error: event already linked with callback. ✗"));
    }
  }
  event_callbacks[event_mask].push_back(callback);
  FW_LOG("[DEBUG] Event linked: event_mask = " + to_string(event_mask) +
         " to callback. ✓");
  return Result<void>::Ok();
}

Result<void> FileWatcher::unlink_event(uint32_t event_mask,
                                       WatchCallback callback) {
  auto bucket = event_callbacks.find(event_mask);
  if (bucket == event_callbacks.end()) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EVENT_NOT_FOUND, "Error: no such event. ✗"));
  }

  for (auto it = bucket->second.begin(); it != bucket->second.end(); it++) {
    if (*it == callback) {
      bucket->second.erase(it);
      FW_LOG("[DEBUG] Event unlinked: event_mask = " +
             to_string(event_mask) + " from callback. ✓");
      return Result<void>::Ok();
    }
  }
  return Result<void>::Err(FWError::make(ErrorCode::CALLBACK_NOT_FOUND,
                                         "Error: callback not found. ✗"));
}

// Polls the inotify fd until stop() flips `watching` off, debouncing bursts
// of events so a single save doesn't fire the linked callbacks repeatedly.
Result<void> FileWatcher::event_loop(int timeout) {
  auto last_dispatch = std::chrono::steady_clock::now();
  const auto debounce_window = std::chrono::milliseconds(300);

  FW_LOG("[DEBUG] Event loop started. ✓");
  while (watching) {
    int ready = poll(poll_targets, poll_target_count, timeout);

    if (ready == -1) {
      if (errno == EINTR)
        continue;
      return Result<void>::Err(FWError::make(ErrorCode::SYS_POLL_FAILED,
                                             "Error: polling error. ✗"));
    }

    if (ready < 0)
      continue;

    if (poll_targets[0].revents & POLLIN) {
      auto now = std::chrono::steady_clock::now();
      if (now - last_dispatch < debounce_window)
        continue;
      last_dispatch = now;

      FW_LOG("[DEBUG] POLLIN received. ✓");
      WatchEvent event;
      vector<WatchCallback> callbacks_to_run;
      {
        lock_guard<mutex> lock(registry_mutex);

        vector<int> watched_descriptors;
        for (const auto &[wd, path] : watch_registry) {
          watched_descriptors.push_back(wd);
          FW_LOG("[DEBUG] Pushing event on path " + path +
                 " to read_next_event. ✓");
        }
        event = TRY(read_next_event(poll_targets[0].fd, watched_descriptors,
                                    watch_registry.size()),
                    void);
        callbacks_to_run = event_callbacks[event.event_mask];
      }
      for (auto &cb : callbacks_to_run) {
        FW_LOG("[DEBUG] Invoking callback for event " +
               to_string(event.event_mask) + " ...");
        TEST(cb.invoke(event));
        FW_LOG("[DEBUG] Callback executed. ✓");
      }
    }
  }
  return Result<void>::Ok();
}

Result<void> FileWatcher::start(int timeout) {
  FW_LOG("[DEBUG] Starting file watcher...");
  if (timeout < 10) {
    timeout = 10;
  }

  if (watching) {
    return Result<void>::Err(
        FWError::make(ErrorCode::FILEWATCHER_ALREADY_RUNNING,
                      "Error: File watcher already running. ✗"));
  }
  try {
    FW_LOG("[DEBUG] Launching a background thread to watch files ...");
    watching = true;
    background_thread = std::thread(&FileWatcher::event_loop, this, timeout);
  } catch (std::system_error &e) {
    return Result<void>::Err(
        FWError::make(ErrorCode::SYS_THREAD_FAILED,
                      "Error: Starting background thread failed. ✗"));
  }
  FW_LOG("[DEBUG] Background thread launched successfully. ✓");
  FW_LOG("[DEBUG] File watcher started successfully. ✓");
  return Result<void>::Ok();
}

Result<void> FileWatcher::stop() {
  if (!watching) {
    return Result<void>::Err(
        FWError::make(ErrorCode::FILEWATCHER_NOT_RUNNING,
                      "Error: File watcher not running. ✗"));
  }
  watching = false;
  FW_LOG("[DEBUG] Stopping file watcher...");
  FW_LOG("[DEBUG] Joining background thread...");
  if (background_thread.joinable()) {
    background_thread.join();
  }
  FW_LOG("[DEBUG] Background thread joined. ✓");
  FW_LOG("[DEBUG] File watcher stopped successfully. ✓");
  return Result<void>::Ok();
}

Result<vector<string>> FileWatcher::get_watch_list() {
  lock_guard<mutex> lock(registry_mutex);
  vector<string> list;
  for (auto &[wd, path] : watch_registry) {
    list.push_back(path);
  }
  return Result<vector<string>>::Ok(list);
}
} // namespace kilket
