#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <sstream>
#include <ctime>
#include <filesystem>

#include "include/session_logger.h"
#include "error/error.h"
#include "include/macros.hpp"

namespace fs = std::filesystem;
// Error
using namespace std;
namespace kilket
{

    Result<SessionLogger*> SessionLogger::create(const string &path)
    {
        FW_LOG("[DEBUG] Creating session logger ...");
        auto logger = new SessionLogger();
        TEST_OVERLOADED(logger->init(path), SessionLogger*);
        FW_LOG("[DEBUG] Session logger created successfully. ✓");
        return Result<SessionLogger*>::Ok(logger);
    }

    Result<void> SessionLogger::init(const string &path)
    {
        // check if file path is empty
        if (path.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: file path is empty " + path + ". ✗"));
        }
        file_path = path;
        FW_LOG("[DEBUG] Log file path set to: " + file_path);
        return Result<void>::Ok();
    }

    SessionLogger::~SessionLogger()
    {
        FW_LOG("[DEBUG] Session logger destroyed.");
        stop();
    }

    Result<void> SessionLogger::start()
    {
        if (is_running)
        {
            return Result<void>::Ok(); // idempotent
        }
