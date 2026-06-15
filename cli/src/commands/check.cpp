#include "../../src/include/kilket_core.h"
#include "../include/CLI11.hpp"
#include "../../src/include/macros.hpp"

namespace fs = std::filesystem;

namespace kilket_cli {
    void register_check(CLI::App* app, kilket::KilketCore* core)
    {
        auto* check = app->add_subcommand("check", "check the status of the task");

        // this is a purely ceremonial flag(so that CLI11 won't say there is unrecognized flag)
        // the actual parsing of verbose and setting of the environment variable is done in main.cpp
        check->add_flag("--debug", KILKET_DEBUG, "Enable debug output. \n// is an extremely detailed output that shows every step of the process.");
        check->add_flag("--verbose", KILKET_VERBOSE, "Enable verbose output. \n// shows a summary of the process.");

        static bool active = false;
        check->add_flag("--active", active, "Check if task is labeled as active");

        static bool deactive = false;
        check->add_flag("--deactive", deactive, "Check if task is labeled as deactive");

        static bool depth;
        check->add_flag("--depth", depth, "Outputs the watching depth set from your working directory.");

        check->callback([=]() mutable {
          std::string check_task_id = fs::current_path().string();

          if(!active && !deactive && !depth) {
              if(!core->is_task(check_task_id)) {
                  std::cout << "Kilket task has not been initialized in the current directory." << std::endl;
                  return;
              }else {
                  std::cout << "Kilket task is initialized in the current directory." << std::endl;
                  return;
              }
          }
