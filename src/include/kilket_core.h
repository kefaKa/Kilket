#pragma once
#include <string>
#include <vector>


#include "task_runner.h"
#include "config_manager.h"
#include "error/result.h"
#include "types.h"

namespace kilket {
    class KilketCore {
        private:
            ConfigManager* config_manager;
            std::vector<TaskRunner*> task_runners;

            std::vector<std::string> default_ignored_patterns;
            std::vector<std::string> default_ignored_paths;


            KilketCore() = default;

            bool isValidDir(std::string &path);
            Result<void> init();

            // Returns the TaskRunner registered under task_id, or nullptr if
            // no such task exists.
            TaskRunner *find_task_runner(const std::string &task_id);
        public:
            static Result<KilketCore*> create();
            ~KilketCore();

            Result<void> set_default_ignored();
            Result<void> set_depth(const std::string &task_id, int depth);
            Result<void> create_task(const std::string &task_name, const std::string &task_id);
            Result<void> delete_task(const std::string &task_id);
            bool is_task(const std::string &task_id);

            std::vector<std::string> get_resolved_files(const std::string task_id);
            Result<bool> is_task_active(const std::string &task_id);
            Result<int> get_task_depth(const std::string &task_id);

            Result<void> start_task(const std::string &task_id);
            Result<void> stop_task(const std::string &task_id);


            Result<void> set_task_path(const std::string &task_id, const std::string &path);
            Result<void> delete_task_path(const std::string &task_id, const std::string &path);

            Result<void> set_task_command(const std::string &task_id, const std::string &command);
            Result<void> delete_task_command(const std::string &task_id, const std::string &command);
