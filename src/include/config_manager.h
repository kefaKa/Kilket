#pragma once
#include <fstream>

#include "error/result.h"
#include "json.hpp"
#include "types.h"

namespace kilket
{
    // Owns the on-disk ~/.config/kilket/config.json: loads it on
    // construction, mutates the in-memory nlohmann::json tree, and flushes
    // it back to disk on demand (or on destruction, if not already flushed).
    class ConfigManager {
        #ifdef KILKET_TESTING
            friend class ConfigManagerTest;
        #endif
        private:
            nlohmann::json config_obj = nlohmann::json::object();
