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
