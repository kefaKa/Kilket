#include "../../src/include/kilket_core.h"
#include "../include/CLI11.hpp"
#include "../../src/include/macros.hpp"

#include <climits>

namespace fs = std::filesystem;

namespace kilket_cli {
    void register_set(CLI::App* app, kilket::KilketCore* core)
    {
        auto* set = app->add_subcommand("set", "setting a property for the task");

        // this is a purely ceremonial flag(so that CLI11 won't say there is unrecognized flag)
        // the actual parsing of verbose and setting of the environment variable is done in main.cpp
        set->add_flag("--debug", KILKET_DEBUG, "Enable debug output. \n// is an extremely detailed output that shows every step of the process.");
        set->add_flag("--verbose", KILKET_VERBOSE, "Enable verbose output. \n// shows a summary of the process.");

        static bool active = false;
        set->add_flag("--active", active, "Set task as active \n // enables you to run specified tasks with the `run active` command");

        static bool deactive = false;
        set->add_flag("--deactive", deactive, "Set a task enables active as deactive");

        static int depth = INT_MIN;
        set->add_option("--depth", depth, "Set the depth to watch from your working directory.");

        set->callback([=]() mutable {
          std::string set_task_id = fs::current_path().string();
