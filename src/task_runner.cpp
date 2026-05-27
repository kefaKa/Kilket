#include <chrono>
#include <filesystem>
#include <fnmatch.h>
#include <stdio.h>
#include <string>
#include <sys/inotify.h>
#include <vector>

#include "error/error.h"
#include "error/result.h"
#include "include/macros.hpp"
#include "include/task_runner.h"

namespace fs = std::filesystem;
using namespace std;

namespace kilket {
Result<TaskRunner *> TaskRunner::create(const string &task_name,
                                        const string &working_directory) {
  TaskRunner *t = new TaskRunner();
  TEST_OVERLOADED(t->init(task_name, working_directory), TaskRunner *);
  return Result<TaskRunner *>::Ok(t);
}

Result<void> TaskRunner::init(const string &task_name,
                              const string &working_directory) {
  FW_LOG("[DEBUG] Initializing task runner for " + task_name + " in " +
         working_directory + " ...✗");
  watcher = TRY(FileWatcher::create(), void);
  task.name = task_name;
  task.id = working_directory;
  flushed = false;
  task.isRunning = false;

  if (!fs::exists(working_directory)) {
    return Result<void>::Err(FWError::make(
        ErrorCode::PATH_NOT_FOUND, "Error: working directory does not exist " +
                                       working_directory + ". ✗"));
  }

  string file_name = task_name + "-kilket.log";
  fs::path _file_path = fs::path(working_directory) / file_name;
  logger = TRY(SessionLogger::create(_file_path.string()), void);

  FW_LOG("[KILKET] Adding path " << _file_path << " to session logger...");

  last_executed =
      std::chrono::steady_clock::now() - std::chrono::milliseconds(500);
  FW_LOG("[DEBUG] Task runner initialized. ✓");
  return Result<void>::Ok();
}

TaskRunner::~TaskRunner() {
  if (watcher)
    watcher->stop();
  delete watcher;
  if (logger)
    logger->stop();
  delete logger;
  task.isRunning = false;
  FW_LOG("[DEBUG] TaskRunner destroyed.");
}

Result<void> TaskRunner::set_depth(int num) {
  if (num > 10) {
    return Result<void>::Err(FWError::make(
        ErrorCode::INVALID_DEPTH, "Error: invalid depth set - depth too much "
                                  "✗, use a depth between 0 and 10"));
  } else if (num < -1) {
    return Result<void>::Err(FWError::make(
        ErrorCode::INVALID_DEPTH, "Error: invalid depth set - depth set too "
                                  "low ✗, use a depth between 0 and 10"));
  }

  task.watching_depth = num;
  FW_LOG("[DEBUG] Depth set to " + std::to_string(num));
  return Result<void>::Ok();
}

Result<void> TaskRunner::change_task_name(const string &task_name) {
  // check if task name is empty
  if (task_name.empty()) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EMPTY_VALUE, "Error: task name is empty ✗"));
  }
  // check if task name already exists
  if (task.name == task_name) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_ALREADY_EXISTS, "Error: task name already exists ✗"));
  }
  task.name = task_name;
  FW_LOG("[DEBUG] Changing task name successful. ✓");
  return Result<void>::Ok();
}

Result<void>
TaskRunner::change_working_directory(const string &working_directory) {
  // check if working directory is empty
  if (working_directory.empty()) {
    return Result<void>::Err(FWError::make(
        ErrorCode::EMPTY_VALUE, "Error: working directory is empty ✗"));
  }
  // check if working directory is a valid path
  if (!fs::exists(working_directory)) {
    return Result<void>::Err(FWError::make(
        ErrorCode::PATH_NOT_FOUND, "Error: working directory not found ✗"));
  }
  // check if working directory already exists
  if (task.id == working_directory) {
    return Result<void>::Err(
        FWError::make(ErrorCode::PATH_ALREADY_EXISTS,
                      "Error: working directory already exists ✗"));
  }
  task.id = working_directory;
  FW_LOG("[DEBUG] Working directory changed successfully. ✓");
  return Result<void>::Ok();
}

bool TaskRunner::isIgnored(const string &path) {
  string filename = fs::path(path).filename().string();
  for (auto &p : task.ignored_paths) {
    if (p == path || p == filename)
      return true;
  }

  for (auto &p : task.ignored_patterns) {
    if (fnmatch(p.c_str(), filename.c_str(), 0) == 0)
      return true;
    if (fnmatch(p.c_str(), path.c_str(), 0) == 0)
      return true;
  }
  return false;
}

Result<void> TaskRunner::add_ignored_path(const string &path) {
  if (path.empty()) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EMPTY_VALUE, "Error: command is empty"));
  }
  for (auto &p : task.ignored_paths) {
    if (p == path) {
      return Result<void>::Err(FWError::make(ErrorCode::PATH_ALREADY_EXISTS,
                                             "Error: path already exists"));
    }
  }
  task.ignored_paths.push_back(path);
  FW_LOG("[DEBUG] Adding ignored path " + path + " to Task completed. ✓");
  return Result<void>::Ok();
}

Result<void> TaskRunner::add_ignored_pattern(const string &pattern) {
  if (pattern.empty()) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EMPTY_VALUE, "Error: command is empty"));
  }
  for (auto &p : task.ignored_patterns) {
    if (p == pattern) {
      return Result<void>::Err(FWError::make(ErrorCode::PATH_ALREADY_EXISTS,
                                             "Error: path already exists"));
    }
  }
  task.ignored_patterns.push_back(pattern);
  FW_LOG("[DEBUG] Adding ignored pattern " + pattern + " to Task completed. ✓");
  return Result<void>::Ok();
}

Result<void> TaskRunner::remove_ignored_path(const string &path) {
  for (auto it = task.ignored_paths.begin(); it != task.ignored_paths.end();
       it++) {
    if (*it == path) {
      task.ignored_paths.erase(it);
      return Result<void>::Ok();
    }
  }

  return Result<void>::Err(FWError::make(ErrorCode::VALUE_NOT_FOUND,
                                         "Error: ignored path not found"));
}

Result<void> TaskRunner::remove_ignored_pattern(const string &pattern) {
  for (auto it = task.ignored_patterns.begin();
       it != task.ignored_patterns.end(); it++) {
    if (*it == pattern) {
      task.ignored_patterns.erase(it);
      return Result<void>::Ok();
    }
  }

  return Result<void>::Err(FWError::make(ErrorCode::VALUE_NOT_FOUND,
                                         "Error: ignored pattern not found"));
}

Result<void> TaskRunner::add_command(const string &command) {
  // check if command is empty
  if (command.empty()) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EMPTY_VALUE, "Error: command is empty"));
  }

  // check if command already exists
  for (auto &cmd : task.commands) {
    if (cmd == command) {
      return Result<void>::Err(
          FWError::make(ErrorCode::COMMAND_ALREADY_EXISTS,
                        "Error: command already exists ✗"));
    }
  }

  task.commands.push_back(command);
  return Result<void>::Ok();
}

Result<void> TaskRunner::delete_command(const string &command) {
  for (auto it = task.commands.begin(); it != task.commands.end(); it++) {
    if (*it == command) {
      task.commands.erase(it);
      return Result<void>::Ok();
    }
  }

  return Result<void>::Err(
      FWError::make(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found"));
}

bool TaskRunner::check_path_existence(const string &path) {
  for (auto &p : task.file_paths) {
    if (p == path) {
      return true;
    }
  }
  for (auto &p : task.dir_paths) {
    if (p == path) {
      return true;
    }
  }
  return false;
}

Result<void> TaskRunner::add_path(const string &path) {
  if (check_path_existence(path))
    return Result<void>::Ok(); // idempotent
  if (!fs::exists(path)) {
    return Result<void>::Err(
        FWError::make(ErrorCode::PATH_NOT_FOUND,
                      "Error: provided path " + path + " does not exist! ✗ \n // use the `kilket remove --path <path>` command to remove it from the task"));
  }
  if (isIgnored(path)) {
    FW_LOG("[DEBUG] Path " + path + " matches ignored paths and patterns.");
    return Result<void>::Ok();
  }

  TEST(add_path_internal(path, task.watching_depth, 0));
  if (fs::is_directory(path)) {
    task.dir_paths.push_back(path);
    FW_VERBOSE("[KILKET] Adding " + path +
               "'s children to task runner completed. ✓");
  } else
    task.file_paths.push_back(path);
  FW_VERBOSE("[KILKET] Adding path " + path + " to task runner completed. ✓");
  return Result<void>::Ok();
}

Result<void> TaskRunner::add_path_internal(const string &path, int max_depth,
                                           int current_depth) {
  if (check_path_existence(path))
    return Result<void>::Ok(); // idempotent
  // if path is directory add each files inside it iteratively
  if (isIgnored(path)) {
    FW_LOG("[DEBUG] Path " + path + " matches ignored paths and patterns.");
    FW_LOG("[DEBUG] Adding Path " + path + " to filewatcher failed. ✗");
    return Result<void>::Ok();
  }

  if (fs::is_directory(path)) {
    FW_LOG("[DEBUG] Path " + path +
           " is a directory adding child files recursively...");
    for (auto &entry : fs::directory_iterator(path)) {
      if (isIgnored(entry.path().string())) {
        FW_LOG("[DEBUG] Path " + entry.path().string() +
               " matches ignored patterns.");
        FW_LOG("[DEBUG] Adding Path " + entry.path().string() +
               " to task failed. ✗");
        continue;
      }

      if (entry.is_regular_file()) {
        FW_LOG("[DEBUG] Adding path " + entry.path().string() +
               " to filewatcher...");
        FW_LOG("[DEBUG] Checking if resolved file path " +
               entry.path().string() +
               " matches ignored paths and patterns ...");

        resolved_files.push_back(entry.path().string());
        watcher->add_path(entry.path().string());

        FW_LOG("[DEBUG] Adding path " + entry.path().string() +
               " to task completed. ✓");
      } else if (entry.is_directory()) {
        if (max_depth > current_depth) {
          resolved_files.push_back(entry.path().string());
          TEST(add_path_internal(entry.path().string(), max_depth,
                                 current_depth + 1));
        } else {
          FW_LOG("[DEBUG] Path " + entry.path().string() +
                 " is a directory. But max_depth=" + to_string(max_depth) +
                 " have been reached. Child files won't be watched.");
        }
      }
    }
  } else if (!fs::is_directory(path)) {
    FW_LOG("[DEBUG] Path " + path + " is a file. Adding to task...");
    resolved_files.push_back(path);
    watcher->add_path(path);
  }
  return Result<void>::Ok();
}

Result<void> TaskRunner::delete_path(const string &path) {
  if (!fs::exists(path)) {
    return Result<void>::Err(
        FWError::make(ErrorCode::PATH_NOT_FOUND,
                      "Error: provided path " + path + " does not exist! ✗"));
  }

  if (!fs::is_directory(path)) {
    for (auto it = task.file_paths.begin(); it != task.file_paths.end(); it++) {
      if (*it == path) {
        task.file_paths.erase(it);
        return Result<void>::Ok();
      }
    }
  } else {
    for (auto it = task.dir_paths.begin(); it != task.dir_paths.end(); it++) {
      if (*it == path) {
        task.dir_paths.erase(it);
        return Result<void>::Ok();
      }
    }
  }
  TEST(watcher->remove_path(path));
  return Result<void>::Err(
      FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: path not found"));
}

Result<void> TaskRunner::add_on_success(const string &command) {
  // check if command is empty
  if (command.empty()) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EMPTY_VALUE, "Error: command is empty"));
  }

  // check if command already exists
  for (auto it = task.on_success.begin(); it != task.on_success.end(); it++) {
    if (*it == command) {
      return Result<void>::Err(FWError::make(ErrorCode::COMMAND_ALREADY_EXISTS,
                                             "Error: command already exists"));
    }
  }
  task.on_success.push_back(command);
  return Result<void>::Ok();
}

Result<void> TaskRunner::delete_on_success(const string &command) {
  for (auto it = task.on_success.begin(); it != task.on_success.end(); it++) {
    if (*it == command) {
      task.on_success.erase(it);
      return Result<void>::Ok();
    }
  }

  return Result<void>::Err(
      FWError::make(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found"));
}
