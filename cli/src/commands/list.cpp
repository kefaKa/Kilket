#include "../../src/include/kilket_core.h"
#include "../include/CLI11.hpp"
#include "../../src/include/macros.hpp"
#include "../../src/include/types.h"

#include <vector>

namespace fs = std::filesystem;
namespace kilket_cli {
    void register_list(CLI::App* app, kilket::KilketCore* core)
    {
        auto* list = app->add_subcommand("list", "listing all tasks, paths, commands, and ignored patterns");

        // this is a purely ceremonial flag(so that CLI11 won't say there is unrecognized flag)
        // the actual parsing of verbose and setting of the environment variable is done in main.cpp
        list->add_flag("--debug", KILKET_DEBUG, "Enable debug output. \n // is an extremely detailed output that shows every step of the process.");
        list->add_flag("--verbose", KILKET_VERBOSE, "Enable verbose output. \n// shows a summary of the process.");

        static std::string list_task_id = "";
        static bool list_tasks = false;
        static bool list_paths = false;
        static bool list_commands = false;
        static bool list_ignored = false;
        static bool list_on_success = false;
        static bool list_on_failure = false;

        list->add_flag("--tasks", list_tasks, "List all tasks registered.");
        list->add_flag("--paths", list_paths, "List all paths registered to be watched in the current directory.");
        list->add_flag("--commands", list_commands, "List all build commands registered in the current directory.");
        list->add_flag("--ignored", list_ignored, "List all ignored paths and patterns in the current directory.");
        list->add_flag("--on-success", list_on_success, "List all on success commands to run in the current directory.");
        list->add_flag("--on-failure", list_on_failure, "List all on failure commands to run in the current directory.");

        list->callback([=]() mutable {
            list_task_id = fs::current_path().string();
            std::vector<kilket::Task> tasks = core->get_tasks();
            kilket::Task* matching_task = nullptr;
            for(auto& task : tasks) {
                if(task.id == list_task_id) {
                    matching_task = &task;
                    break;
                }
            }

            if (list_tasks) {
                std::cout << "\nAll registered Tasks:\n" << std::endl;
                for (const auto& task : tasks) {
                    std::cout << " Name: " << task.name << std::endl;
                    std::cout << " Working Directory: " << task.id << "\n" << std::endl;
                }
            }

            if (list_paths) {
                if(matching_task == nullptr) {
                    std::cout << "Error: No kilket task found for the current directory." << std::endl;
                    return;
                }

                std::cout << "\nWatched Directories:\n" << std::endl;
                for(const auto& path : matching_task->dir_paths) {
                    std::cout << " " << path << std::endl;
                }

                std::cout << "\nStandalone File:\n" << std::endl;
                for(const auto& path : matching_task->file_paths) {
                    std::cout << " " << path << std::endl;
                }

                std::cout << "All Currently Watched Files:\n" << std::endl;
                for(const auto& path : core->get_resolved_files(matching_task->id)) {
                    std::cout << " " << path << std::endl;
                }

            }

            if (list_commands) {
                if(matching_task == nullptr) {
                    std::cout << "Error: No kilket task found for the current directory." << std::endl;
                    return;
                }

                std::cout << "\nCommands:\n" << std::endl;
                for(const auto& command : matching_task->commands) {
                    std::cout << " " << command << std::endl;
                }
            }

            if (list_ignored) {
                if(matching_task == nullptr) {
                    std::cout << "Error: No kilket task found for the current directory." << std::endl;
                    return;
                }

                std::cout << "\nIgnored paths:\n" << std::endl;
                for(const auto& ignored_paths : matching_task->ignored_paths) {
                    std::cout << " " << ignored_paths << std::endl;
                }
                std::cout << "\nIgnored patterns:\n" << std::endl;
                for(const auto& ignored_pattern : matching_task->ignored_patterns) {
                    std::cout << " " << ignored_pattern << std::endl;
                }
            }

            if (list_on_success) {
                if(matching_task == nullptr) {
                    std::cerr << "Error: No kilket task found for the current directory." << std::endl;
                    return;
                }

                std::cout << "\nOn success:\n" << std::endl;
                for(const auto& command : matching_task->on_success) {
                    std::cout << " " << command << std::endl;
                }
                std::cout << std::endl;
            }

            if (list_on_failure) {
                if(matching_task == nullptr) {
                    std::cerr << "Error: No kilket task found for the current directory." << std::endl;
                    return;
                }

                std::cout << "\nOn failure:\n" << std::endl;
                for(const auto& command : matching_task->on_failure) {
                    std::cout << " " << command << std::endl;
                }
                std::cout << std::endl;
            }
        });
    }
}
