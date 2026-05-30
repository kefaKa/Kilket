#pragma once

#include <vector>
#include <string>

#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>

#include "error/result.h"
#include "types.h"

namespace kilket
{
    // Thin wrapper around a Linux inotify instance: tracks which paths are
    // being watched, dispatches matching events to linked callbacks, and
    // runs the poll loop on a background thread.
    class FileWatcher
    {
    private:
        int inotify_fd = -1;

        // wd -> path and its inverse, so lookups work in both directions.
        std::unordered_map<int, std::string> watch_registry;
        std::unordered_map<std::string, int> reverse_watch_registry;

        struct pollfd poll_targets[1];
        nfds_t poll_target_count;

        std::unordered_map<uint32_t, std::vector<WatchCallback>> event_callbacks;

        std::mutex registry_mutex;
        std::atomic<bool> watching{false};
        std::thread background_thread;

        FileWatcher() = default;
        Result<void> init();

        Result<WatchEvent> read_next_event(int fd, std::vector<int> watched_descriptors, int descriptor_count);
        Result<void> event_loop(int timeout);

    public:
        static Result<FileWatcher *> create();
