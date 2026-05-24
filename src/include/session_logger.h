#pragma once

#include <string>
#include <fstream>

#include "json.hpp"
#include "error/result.h"
#include "types.h"

namespace kilket
{
    using json = nlohmann::json;

    // Buffers execution results for one task into a JSON session object and
    // writes it to `<task>-kilket.log` when the session stops.
    class SessionLogger {
        private:
            std::fstream file;
            std::string file_path;
            json session = json::object();

            bool is_running = false;
            bool flushed = false;
            SessionLogger() = default;
            Result<void> init(const std::string &path);
        public:
            static Result<SessionLogger*> create(const std::string &path);
            ~SessionLogger();

            Result<void> start();
            Result<void> log_execution(const ExecutionResult &execution_result);
            Result<void> stop();
    };
}
