#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "error/result.h"

namespace kilket
{
    class TaskRunner;

    // A single filesystem change reported by inotify, normalized into
    // something the rest of the pipeline can reason about.
    struct WatchEvent
    {
        int wd = -1;
        std::string filetype = "";
        std::string path = "";
        uint32_t event_mask = -1;

        WatchEvent() = default;

        WatchEvent(int watch_descriptor, std::string type, std::string changed_path, uint32_t mask)
            : wd(watch_descriptor), filetype(std::move(type)), path(std::move(changed_path)), event_mask(mask) {}

        bool isNull() const
        {
            return wd == -1 && filetype.empty() && path.empty() && event_mask == -1;
        }
    };

    // Either a free function or a bound member function that gets fired
    // whenever a linked event_mask fires. Only one of the two is ever set.
    struct WatchCallback
    {
        TaskRunner *ptr = nullptr;
        Result<void> (TaskRunner::*handler)(const WatchEvent &e) = nullptr;
        Result<void> (*raw_callback)(const WatchEvent &e) = nullptr;

        static WatchCallback from_raw(Result<void> (*fn)(const WatchEvent &e))
        {
            WatchCallback cb;
            cb.ptr = nullptr;
            cb.handler = nullptr;
            cb.raw_callback = fn;
            return cb;
        }

        bool operator==(const WatchCallback &other) const
        {
            return ptr == other.ptr && handler == other.handler && raw_callback == other.raw_callback;
        }

        Result<void> invoke(const WatchEvent &e) const
        {
            if (ptr == nullptr)
                return raw_callback(e);
            return (ptr->*handler)(e);
        }

        bool isNull() const
        {
            return ptr == nullptr && handler == nullptr && raw_callback == nullptr;
        }
    };

    // A registered watch target: what to build, what paths feed it, and
    // what to run afterward depending on the exit code.
    struct Task
    {
        // Internal, always-unique identifier — not user-facing. Usually the
        // absolute path of the working directory the task was created in.
        std::string id = "";
        // User-facing label; may be duplicated across tasks. Defaults to the
        // basename of the working directory.
        std::string name = "";
        int watching_depth = 3;

        std::vector<std::string> commands = {};
        std::vector<std::string> file_paths = {};
        std::vector<std::string> dir_paths = {};

        std::vector<std::string> on_success = {};
        std::vector<std::string> on_failure = {};

        std::vector<std::string> ignored_patterns = {};
        std::vector<std::string> ignored_paths = {};

        bool isActive = false;
        bool isRunning = false;

        bool isNull() const
        {
            return id.empty() && name.empty() && commands.empty() && file_paths.empty() && dir_paths.empty() &&
                   on_success.empty() && on_failure.empty() && !isActive && !isRunning && watching_depth == 3 &&
                   ignored_paths.empty() && ignored_patterns.empty();
        }
    };

    // Outcome of running a task's build (and hook) commands once.
    struct ExecutionResult
    {
        int id;
        int exit_code;
        WatchEvent _event;
        std::string log;
        std::vector<std::string> build_commands;

        ExecutionResult() : id(-1), exit_code(-1), log(""), build_commands({}) {}

        ExecutionResult(int run_id, int code, WatchEvent event = WatchEvent(), std::string output = "",
                        std::vector<std::string> commands = {})
            : id(run_id), exit_code(code), _event(std::move(event)), log(std::move(output)),
              build_commands(std::move(commands)) {}
    };
}
