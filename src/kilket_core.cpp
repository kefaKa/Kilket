#include <filesystem>
#include <string>
#include <vector>

#include "include/error/error.h"
#include "include/error/result.h"
#include "include/kilket_core.h"
#include "include/macros.hpp"
#include "include/task_runner.h"

using namespace std;
namespace fs = std::filesystem;

namespace kilket {

Result<KilketCore *> KilketCore::create() {
  KilketCore *core = new KilketCore();
  TEST_OVERLOADED(core->init(), KilketCore *);
  return Result<KilketCore *>::Ok(core);
}

KilketCore::~KilketCore() {
  config_manager->flush();
  for (auto *runner : task_runners) {
    delete runner;
  }
}

Result<void> KilketCore::set_default_ignored() {
  default_ignored_patterns = {"*.o",   "*.a",   "*.so", "*.out", "*.exe",
                              "*.swp", "*.swo", "*~",   ".#*",   "*.class",
                              "*.pyc", "*.log", "*.git"};
  default_ignored_paths = {".git"};
  return Result<void>::Ok();
}

// Finds the TaskRunner owning task_id, or nullptr — every mutating method
// below funnels through this instead of repeating the same linear scan.
TaskRunner *KilketCore::find_task_runner(const std::string &task_id) {
  for (auto *runner : task_runners) {
    if (runner->get_task_id() == task_id)
      return runner;
  }
  return nullptr;
}

Result<void> KilketCore::init() {
  config_manager = TRY(ConfigManager::create(), void);
  FW_LOG("[DEBUG] initializing a Kilket core instance ....");

  vector<Task> tasks = TRY(config_manager->get_tasks(), void);
  set_default_ignored();

  FW_LOG("[DEBUG] loading tasks from config file ....");
  for (auto &task : tasks) {
    TaskRunner *runner = TRY(TaskRunner::create(task.name, task.id), void);
    for (auto &path : task.ignored_paths)
      TEST(runner->add_ignored_path(path));
    for (auto &pattern : task.ignored_patterns)
      TEST(runner->add_ignored_pattern(pattern));

    if (task.isActive)
      runner->activate();
    else
      runner->deactivate();
    runner->set_depth(task.watching_depth);

    for (auto &command : task.commands)
      TEST(runner->add_command(command));

    for (auto &path : task.file_paths) {
      auto result = runner->add_path(path);
      if (result.isErr()) {
        if (result.getErrCode() == ErrorCode::PATH_NOT_FOUND)
          runner->delete_path(path);
        else
          return Result<void>::Err(result.unwrapErr());
      }
    }
    for (auto &path : task.dir_paths) {
      auto result = runner->add_path(path);
      if (result.isErr()) {
        if (result.getErrCode() == ErrorCode::PATH_NOT_FOUND)
          runner->delete_path(path);
        else
          return Result<void>::Err(result.unwrapErr());
      }
    }

    for (auto &command : task.on_success)
      TEST(runner->add_on_success(command));
    for (auto &command : task.on_failure)
      TEST(runner->add_on_failure(command));

    task_runners.push_back(runner);
  }
  FW_LOG("[DEBUG] loading tasks from config file completed. ✓");
  FW_VERBOSE("[KILKET] Kilket core initialized.");
  return Result<void>::Ok();
}

Result<void> KilketCore::create_task(const std::string &task_name,
                                       const std::string &task_id) {
  FW_LOG("[DEBUG] Creating task...");
  if (task_name.empty()) {
    return Result<void>::Err(FWError::make(
        ErrorCode::EMPTY_VALUE, "Error: task name cannot be empty ✗"));
  }
  if (find_task_runner(task_id) != nullptr) {
    return Result<void>::Err(FWError::make(ErrorCode::DUPLICATE_ENTRY,
                                           "Error: task already exists ✗"));
  }

  if (!fs::exists(task_id) || !fs::is_directory(task_id)) {
    return Result<void>::Err(FWError::make(
        ErrorCode::PATH_NOT_FOUND, "Error: working directory not found ✗"));
  }

  if (task_runners.size() >= 200) {
    return Result<void>::Err(
        FWError::make(ErrorCode::TASK_FULL, "Error: task limit reached ✗"));
  }
  auto runner = TRY(TaskRunner::create(task_name, task_id), void);

  for (auto &path : default_ignored_paths)
    runner->add_ignored_path(path);
  for (auto &pattern : default_ignored_patterns)
    runner->add_ignored_pattern(pattern);

  task_runners.push_back(runner);
  TEST(config_manager->log_task(runner->get_task()));

  FW_LOG("[DEBUG] logging task to config file completed. ✓");
  FW_VERBOSE("[KILKET] Task created: name='" + task_name + "' path='" +
             task_id + "' ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::delete_task(const std::string &task_id) {
  FW_LOG("[DEBUG] Deleting task...");
  for (auto it = task_runners.begin(); it != task_runners.end(); it++) {
    if ((*it)->get_task_id() == task_id) {
      Task removed_task = (*it)->get_task();
      TEST(config_manager->delete_task(removed_task));
      task_runners.erase(it);
      FW_VERBOSE("[KILKET] Task deleted: " + task_id + "' ✓");
      return Result<void>::Ok();
    }
  }

  return Result<void>::Err(FWError::make(
      ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
}

bool KilketCore::is_task(const std::string &task_id) {
  return find_task_runner(task_id) != nullptr;
}

Result<void> KilketCore::set_depth(const std::string &task_id, int depth) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  TEST(runner->set_depth(depth));
  config_manager->update_task(runner->get_task());
  return Result<void>::Ok();
}

std::vector<std::string>
KilketCore::get_resolved_files(const std::string task_id) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr)
    return std::vector<std::string>();
  return runner->get_resolved_files();
}

Result<int> KilketCore::get_task_depth(const std::string &task_id) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<int>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  return Result<int>::Ok(runner->get_task().watching_depth);
}

Result<bool> KilketCore::is_task_active(const std::string &task_id) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<bool>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  return Result<bool>::Ok(runner->is_active());
}

Result<void> KilketCore::activate_task(const std::string &task_id) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  runner->activate();
  config_manager->update_task(runner->get_task());
  FW_VERBOSE("[KILKET] Task activated: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::deactivate_task(const std::string &task_id) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  runner->deactivate();
  config_manager->update_task(runner->get_task());
  FW_VERBOSE("[KILKET] Task deactivated: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::set_task_path(const std::string &task_id,
                                         const std::string &path) {
  if (!fs::exists(path)) {
    return Result<void>::Err(FWError::make(
        ErrorCode::PATH_NOT_FOUND, "Error: path not found " + path + " ✗"));
  }
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  TEST(runner->add_path(path));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Task path set: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::delete_task_path(const std::string &task_id,
                                            const std::string &path) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  TEST(runner->delete_path(path));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Task path deleted: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::set_ignored_path(const std::string &task_id,
                                            const std::string &path) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  TEST(runner->add_ignored_path(path));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Task ignore path set: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::set_ignored_pattern(const std::string &task_id,
                                               const std::string &pattern) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗ "));
  }
  TEST(runner->add_ignored_pattern(pattern));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Task ignore pattern set: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::remove_ignored_path(const std::string &task_id,
                                               const std::string &path) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗"));
  }
  TEST(runner->remove_ignored_path(path));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Ignored path removed: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::remove_ignored_pattern(const std::string &task_id,
                                                  const std::string &pattern) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗ "));
  }
  TEST(runner->remove_ignored_pattern(pattern));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Ignored pattern removed: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::set_task_command(const std::string &task_id,
                                            const std::string &command) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗ "));
  }
  TEST(runner->add_command(command));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Task command set: " + task_id + " ✓");
  return Result<void>::Ok();
}

Result<void> KilketCore::delete_task_command(const std::string &task_id,
                                               const std::string &command) {
  TaskRunner *runner = find_task_runner(task_id);
  if (runner == nullptr) {
    return Result<void>::Err(FWError::make(
        ErrorCode::TASK_NOT_FOUND, "Error: task not found " + task_id + " ✗ "));
  }
  TEST(runner->delete_command(command));
  config_manager->update_task(runner->get_task());
  FW_LOG("[DEBUG] Task command deleted: " + task_id + " ✓");
  return Result<void>::Ok();
}
