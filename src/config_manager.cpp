#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdlib>

#include <chrono>
#include <format>

#include "include/config_manager.h"
#include "include/macros.hpp"
#include "version.h"

namespace fs = std::filesystem;
using namespace nlohmann;
using namespace std;

namespace kilket
{
    std::filesystem::path get_config_path()
    {
        const char *override = getenv("KILKET_CONFIG_DIR_TEST");
        const char *home = std::getenv("HOME");

        if (override)
        {
            return std::filesystem::path(override) / "config.json";
        }
        return std::filesystem::path(home) / ".config" / "kilket" / "config.json";
    }

    Result<void> ensure_config_dir()
    {
        std::error_code ec;
        std::filesystem::create_directories(get_config_path().parent_path(), ec);
        if (ec)
        {
            return Result<void>::Err(FWError::make(ErrorCode::SYS_IO_FAILED, ec.message()));
        }
        return Result<void>::Ok();
    }

    Result<ConfigManager *> ConfigManager::create()
    {
        ConfigManager *cm = new ConfigManager();
        auto config_path = get_config_path();
        if (!fs::exists(config_path))
        {
            fs::create_directories(config_path.parent_path());
            ofstream(config_path).close();
            auto now = std::chrono::system_clock::now();
            auto ts = std::format("{:%Y-%m-%dT%H:%M:%SZ}", now);

            cm->config_obj["version"] = KILKET_VERSION;
            cm->config_obj["created_at"] = ts;
            cm->config_obj["last_modified"] = ts;
            cm->config_obj["tasks"] = json::array();
        }
        else
        {
            fstream file;
            file.open(config_path, ios::in | ios::out);
            if (!file.is_open())
            {
                return Result<ConfigManager *>::Err(FWError::make(
                    ErrorCode::SYS_IO_FAILED, "Error: opening config file"));
            }
            TEST_OVERLOADED(cm->load(file), ConfigManager *);
            file.close();
        }
        return Result<ConfigManager *>::Ok(cm);
    }

    ConfigManager::~ConfigManager()
    {
        if (!isflushed)
            flush();
    }

    Result<void> ConfigManager::load(std::fstream &file)
    {
        if (file.peek() == ifstream::traits_type::eof())
        {
            return Result<void>::Ok();
        }
        else
        {
            try {
                config_obj = json::parse(file);
            }
            catch (const json::parse_error& e)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::CONFIG_PARSE_FAILED, "Error: parsing config file failed"));
            }
        }

        if (file.fail())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SYS_IO_FAILED, "Error: reading config file failed"));
        }
        return Result<void>::Ok();
    }

    Result<Task> ConfigManager::convert_json_to_task(const json &json_task)
    {
        Task task_out;
        task_out.name = json_task.at("task_name");
        task_out.id = json_task.at("working_directory");
