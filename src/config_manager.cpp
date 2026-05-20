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

        vector<string> commands_out;
        for (auto &cmd : json_task.at("commands"))
        {
            commands_out.push_back(cmd);
        }
        task_out.commands = commands_out;

        vector<string> file_paths_out;
        for (auto &cmd : json_task.at("file_paths"))
        {
            file_paths_out.push_back(cmd);
        }
        task_out.file_paths = file_paths_out;

        vector<string> dir_paths_out;
        for (auto &cmd : json_task.at("dir_paths"))
        {
            dir_paths_out.push_back(cmd);
        }
        task_out.dir_paths = dir_paths_out;

        vector<string> on_success_out;
        for (auto &cmd : json_task.at("on_success"))
        {
            on_success_out.push_back(cmd);
        }
        task_out.on_success = on_success_out;

        vector<string> on_failure_out;
        for (auto &cmd : json_task.at("on_failure"))
        {
            on_failure_out.push_back(cmd);
        }
        task_out.on_failure = on_failure_out;

        vector<string> ignored_paths_out;
        for (auto &path : json_task.at("ignored_paths"))
        {
            ignored_paths_out.push_back(path);
        }
        task_out.ignored_paths = ignored_paths_out;

        vector<string> ignored_patterns_out;
        for (auto &pattern : json_task.at("ignored_patterns"))
        {
            ignored_patterns_out.push_back(pattern);
        }
        task_out.ignored_patterns = ignored_patterns_out;

        task_out.isActive = json_task.at("isActive");
        task_out.watching_depth = json_task.at("watching_depth");
        return Result<Task>::Ok(task_out);
    }

    Result<json> ConfigManager::convert_task_to_json(const Task &task)
    {
        json json_task = json::object();
        json_task["task_name"] = task.name;
        json_task["working_directory"] = task.id;
        json_task["commands"] = json::array();
        for (auto &cmd : task.commands)
        {
            json_task["commands"].push_back(cmd);
        }
        json_task["file_paths"] = json::array();
        for (auto &path : task.file_paths)
        {
            json_task["file_paths"].push_back(path);
        }

        json_task["dir_paths"] = json::array();
        for (auto &path : task.dir_paths)
        {
            json_task["dir_paths"].push_back(path);
        }

        json_task["on_success"] = json::array();
        for (auto &cmd : task.on_success)
        {
            json_task["on_success"].push_back(cmd);
        }
        json_task["on_failure"] = json::array();
        for (auto &cmd : task.on_failure)
        {
            json_task["on_failure"].push_back(cmd);
        }
        for (auto &path : task.ignored_paths)
        {
            json_task["ignored_paths"].push_back(path);
        }
        for (auto &pattern : task.ignored_patterns)
        {
            json_task["ignored_patterns"].push_back(pattern);
        }
        json_task["isActive"] = task.isActive;
        json_task["watching_depth"] = task.watching_depth;
        return Result<json>::Ok(json_task);
    }

    Result<void> ConfigManager::log_task(const Task &task)
    {
        // check if task is empty
        if (task.isNull())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: task is empty"));
        }
        // check if task already exists
        for (auto &t : config_obj["tasks"])
        {
            Task existing_task = TRY(convert_json_to_task(t), void);
            if (existing_task.id == task.id)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::TASK_ALREADY_EXISTS, "Error: task already exists, use update_task to update it"));
            }
        }

        json json_task = TRY(convert_task_to_json(task), void);
        config_obj["tasks"].push_back(json_task);

        return Result<void>::Ok();
    }

    Result<void> ConfigManager::update_task(const Task &task)
    {
        for (auto it = config_obj["tasks"].begin(); it != config_obj["tasks"].end(); it++)
        {
            string id = it->at("working_directory");
            if (id == task.id)
            {
                json json_task = TRY(convert_task_to_json(task), void);
                *it = json_task;
                isflushed = false;
                flush();
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> ConfigManager::purge_config()
    {
        auto config_path = get_config_path();
        fstream file(config_path, ios::out | ios::trunc);
        if(!file.is_open())
        {
            return Result<void>::Err(FWError::make(ErrorCode::SYS_IO_FAILED, "Error: opening config file failed"));
        }
        file.close();
        config_obj = json::object();
        return Result<void>::Ok();
    }

    Result<void> ConfigManager::log_task_inbatch(const vector<Task> &tasks)
    {
        for (auto &task : tasks)
        {
            TEST(log_task(task));
        }
        return Result<void>::Ok();
    }

    Result<void> ConfigManager::delete_task(const Task &task)
    {
        for (auto it = config_obj["tasks"].begin(); it != config_obj["tasks"].end(); it++)
        {
            string id = it->at("working_directory");
            if (id == task.id)
            {
                config_obj["tasks"].erase(it);
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<vector<Task>> ConfigManager::get_tasks()
    {
        vector<Task> tasks;
        for (auto it = config_obj["tasks"].begin(); it != config_obj["tasks"].end(); it++)
        {
            Task task_out;
            task_out.name = it->at("task_name");
            task_out.id = it->at("working_directory");
            task_out.commands = it->at("commands");
            task_out.file_paths = it->at("file_paths");
            task_out.dir_paths = it->at("dir_paths");
            task_out.on_success = it->at("on_success");
            task_out.on_failure = it->at("on_failure");
            task_out.ignored_paths = it->at("ignored_paths");
            task_out.ignored_patterns = it->at("ignored_patterns");
            task_out.isActive = it->at("isActive");
            task_out.watching_depth = it->at("watching_depth");
            tasks.push_back(task_out);
        }
        return Result<vector<Task>>::Ok(tasks);
    }

    Result<void> ConfigManager::flush()
    {
        if (isflushed)
        {
            return Result<void>::Ok();
        }

        auto config_path = get_config_path();
        if (fs::exists(config_path))
        {
            auto now = std::chrono::system_clock::now();
            auto ts = std::format("{:%Y-%m-%dT%H:%M:%SZ}", now);

            config_obj["last_modified"] = ts;

            fstream file(config_path, ios::in | ios::out | ios::trunc);
            file << config_obj.dump(4) << endl;
            if (file.fail())
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::SYS_IO_FAILED, "Error: writing to config file failed"));
            }
            isflushed = true;
            file.close();
            return Result<void>::Ok();
        } else {
            return Result<void>::Err(FWError::make(
                ErrorCode::SYS_IO_FAILED, "Error: config file not found"));
        }
    }
}
