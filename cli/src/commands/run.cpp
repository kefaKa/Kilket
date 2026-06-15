#include "../../src/include/kilket_core.h"
#include "../../src/include/macros.hpp"
#include "../include/CLI11.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace kilket_cli {
void register_run(CLI::App *app, kilket::KilketCore *core) {
  auto *run = app->add_subcommand("run", "running a task");

  // this is a purely ceremonial flag(so that CLI11 won't say there is
  // unrecognized flag) the actual parsing of verbose and setting of the
  // environment variable is done in main.cpp
  run->add_flag("--debug", KILKET_DEBUG,
                "Enable debug output. \n// is an extremely detailed output "
                "that shows every step of the process.");
  run->add_flag("--verbose", KILKET_VERBOSE,
                "Enable verbose output. \n// shows a summary of the process.");
  run->add_flag("--quiet", KILKET_QUIET,
                "Removes build output from terminal");

  static bool run_all = false;
  run->add_flag("--all", run_all, "Run all task instances labeled as active.");

  static bool run_active = false;
  run->add_flag("--active", run_active, "Run active task instances");

  run->callback([=]() mutable {
    std::string run_task_id = fs::current_path().string();
