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
