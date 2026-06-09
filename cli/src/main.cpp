#include "../../src/include/kilket_core.h"
#include "./include/CLI11.hpp"
#include <iostream>
#include <unistd.h>
#include <filesystem>
#include <cstdlib>
#include <csignal>

#include "../../src/include/macros.hpp"
#include "version.h"

namespace kilket_cli {
    void register_add(CLI::App*, kilket::KilketCore*);
    void register_run(CLI::App*, kilket::KilketCore*);
    void register_init(CLI::App*, kilket::KilketCore*);
    void register_list(CLI::App*, kilket::KilketCore*);
    void register_remove(CLI::App*, kilket::KilketCore*);
    void register_set(CLI::App*, kilket::KilketCore*);
    void register_check(CLI::App*, kilket::KilketCore*);
}

namespace fs = std::filesystem;
using namespace kilket;
bool KILKET_VERBOSE = false;
bool KILKET_DEBUG = false;
bool KILKET_QUIET = false;

static KilketCore* core = nullptr;

void handle_sigint(int)
{
    if(core) core->stop_all();
    std::cout << "Exiting safely..." << std::endl;
    exit(0);
}

fs::path get_home_directory() {
#if defined(_WIN32) || defined(_WIN64)
    // Windows environment
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile) {
        return fs::path(userProfile);
    }

    // Fallback for older Windows environments
    const char* homeDrive = std::getenv("HOMEDRIVE");
    const char* homePath = std::getenv("HOMEPATH");
    if (homeDrive && homePath) {
        return fs::path(homeDrive) / homePath;
    }
#else
    // Linux, WSL, macOS, and other UNIX-like systems
    const char* home = std::getenv("HOME");
    if (home) {
        return fs::path(home);
    }
#endif

    // Return an empty path if nothing was found
    return fs::path();
}

int main(int argc, char **argv) {
    signal(SIGINT, handle_sigint);

  for (int i = 1; i < argc; i++)
  {
      if (std::string(argv[i]) == "--debug")
      {
          KILKET_DEBUG = true;
          KILKET_VERBOSE = true;
      }
      if (std::string(argv[i]) == "--verbose")
          KILKET_VERBOSE = true;

      if (std::string(argv[i]) == "--quiet")
          KILKET_QUIET = true;

      if (std::string(argv[i]) == "--version")
      {
          std::cout << "Kilket version " << KILKET_VERSION << std::endl;
          return 0;
      }
  }

  auto core_result = KilketCore::create();
  if (core_result.isErr()) {
      if(core_result.getErrCode() == ErrorCode::CONFIG_PARSE_FAILED)
      {
          std::cerr << "Config file is corrupted." << std::endl;
          fs::path config_path = get_home_directory() / ".config" / "kilket"/ "config.json";
          std::cout << "Path : " << config_path << std::endl;
          std::cerr << "Options: " << std::endl;
          std::cout << " [1] clear all data and start fresh" << std::endl;
          std::cout << " [2] exit and fix manually" << std::endl;

          std::cout << "Enter your choice: ";
          int choice;
          std::cin >> choice;
          if (choice == 1)
          {
              fs::remove(config_path);
              std::cout << "Config file cleared. Starting fresh..." << std::endl;
              return 0;
          }
          else if (choice == 2)
              return 1;
          else
              std::cerr << "Invalid choice." << std::endl;
          return 1;
      }
      else
      {
          std::cerr << "Failed to create Kilket instance: " << core_result.getErrMessage()
              << std::endl;
          return 1;
      }
  }
  core = core_result.unwrap();

  CLI::App app{"Kilket"};
  app.require_subcommand(0, 1);

  bool version_flag = false;
  app.add_flag("--version", version_flag, "Print version information");

  kilket_cli::register_init(&app, core);
  kilket_cli::register_add(&app, core);
  kilket_cli::register_run(&app, core);
  kilket_cli::register_list(&app, core);
  kilket_cli::register_remove(&app, core);
  kilket_cli::register_set(&app, core);
  kilket_cli::register_check(&app, core);

  CLI11_PARSE(app, argc, argv);
  delete core;
  return 0;
}
