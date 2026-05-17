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
