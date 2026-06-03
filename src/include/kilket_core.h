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
