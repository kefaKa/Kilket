#include "../../src/include/kilket_core.h"
#include "../include/CLI11.hpp"
#include "../../src/include/macros.hpp"
#include <filesystem>

namespace fs = std::filesystem;
namespace kilket_cli {
    void register_init(CLI::App* app, kilket::KilketCore* core)
    {
        // init subcommand
        auto *init =
            app->add_subcommand("init", "initializing a new Kilket task instance in the current folder");

        // this is a purely ceremonial flag(so that CLI11 won't say there is unrecognized flag)
        // the actual parsing of verbose and setting of the environment variable is done in main.cpp
        init->add_flag("--debug", KILKET_DEBUG, "Enable debug output. \n// is an extremely detailed output that shows every step of the process.");
        init->add_flag("--verbose", KILKET_VERBOSE, "Enable verbose output. \n// shows a summary of the process.");

        static std::string task_name = "";
        init->add_option("--task", task_name, "Task name to initialize [optional] \n// filename of the current folder will be taken as task name if not provided");

        init->callback([=]() mutable {
          fs::path cwd = fs::canonical(fs::current_path());
