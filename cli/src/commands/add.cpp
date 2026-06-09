#include "../../src/include/kilket_core.h"
#include "../include/CLI11.hpp"
#include "../../src/include/macros.hpp"

namespace fs = std::filesystem;
namespace kilket_cli {

void register_add(CLI::App *app, kilket::KilketCore *core) {

    // add subcommand
  auto *add =
      app->add_subcommand("add", "adding a new task to the Kilket project \n");

  // this is a purely ceremonial flag(so that CLI11 won't say there is unrecognized flag)
  // the actual parsing of verbose and setting of the environment variable is done in main.cpp
  add->add_flag("--debug", KILKET_DEBUG, "Enable debug output. \n// is an extremely detailed output that shows every step of the process.");
  add->add_flag("--verbose", KILKET_VERBOSE, "Enable verbose output. \n// shows a summary of the process.");

  static std::string task_id = "";
  static std::string add_n_path = "";
  add->add_option("--path", add_n_path, "Path to the task [optional] \n// use to add a new path to be watched.\n");

  static std::string add_command = "";
  add->add_option("--command", add_command, "Command to run for the task [optional] \n// if not provided, the task will not run a command when triggered\n");

  static std::string command_on_success = "";
  static std::string command_on_failure = "";
  static std::string ignored_path = "";
  static std::string ignored_pattern = "";

  add->add_option("--on-success", command_on_success,
                  "Command to run on success [optional] \n// if not provided, no command will be run on success\n");
  add->add_option("--on-failure", command_on_failure,
                  "Command to run on failure [optional] \n// if not provided, no command will be run on failure\n");
  add->add_option("--ignored-path", ignored_path, "Ignored path [optional] \n// if not provided, all files will be watched in the working directory\n");
  add->add_option("--ignored-pattern", ignored_pattern, "Ignored pattern [optional] \n// if not provided, no pattern will be ignored in the working directory\n");
