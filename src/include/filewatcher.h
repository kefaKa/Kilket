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
