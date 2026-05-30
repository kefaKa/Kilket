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
