#pragma once

#include <vector>
#include <string>
#include <chrono>

#include "types.h"
#include "filewatcher.h"
#include "session_logger.h"

namespace kilket
{
    class FileWatcher;

    // Owns one watched project: its FileWatcher instance, its SessionLogger,
    // and the Task record (commands, paths, hooks) that describes it.
    class TaskRunner
    {
    private:
        FileWatcher *watcher = nullptr;
        SessionLogger *logger;
        Task task;
        std::vector<WatchCallback> callbacks;

        std::chrono::steady_clock::time_point last_executed;
        std::chrono::milliseconds cooldown_ms{500};

        bool flushed;
        bool is_init = false;
        int execution_id = 0;
        std::vector<std::string> resolved_files;

        TaskRunner() = default;
        Result<void> init(const std::string &task_name, const std::string &working_directory);
        Result<void> add_path_internal(const std::string &path, int max_depth, int current_depth);
        bool check_path_existence(const std::string &path);
    public:
        static Result<TaskRunner*> create(const std::string &task_name, const std::string &working_directory);
        ~TaskRunner();

        std::string get_task_name() const { return task.name; }
        std::string get_task_id() const { return task.id; }
        Task get_task() const { return task; }
        std::vector<std::string> get_resolved_files() const { return resolved_files; }
