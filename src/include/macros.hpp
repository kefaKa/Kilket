#pragma once

// Unwraps a Result, propagating its error (with a frame appended) out of the
// current function early. Used inside functions that themselves return
// Result<ReturnType>.
#define TRY(expr, ReturnType) ({                        \
    auto _outcome = (expr);                             \
    if (_outcome.isErr())                               \
    {                                                    \
        auto _err = _outcome.unwrapErr();                \
        _err.pushFrame(std::source_location::current()); \
        return Result<ReturnType>::Err(_err);            \
    }                                                    \
    _outcome.unwrap();                                  \
})

// Same as TRY, but for call sites inside a function returning Result<void>
// that don't need the unwrapped value.
#define TEST(expr) ({                                   \
    auto _outcome = (expr);                             \
    if (_outcome.isErr())                               \
    {                                                    \
        auto _err = _outcome.unwrapErr();                \
        _err.pushFrame(std::source_location::current()); \
        return Result<void>::Err(_err);                  \
    }                                                    \
})

// Same as TRY, but discards the unwrapped value instead of returning it —
// for constructors/factories that just need to bail on failure.
#define TEST_OVERLOADED(expr, ReturnType) ({             \
    auto _outcome = (expr);                             \
    if (_outcome.isErr())                               \
    {                                                    \
        auto _err = _outcome.unwrapErr();                \
        _err.pushFrame(std::source_location::current()); \
        return Result<ReturnType>::Err(_err);            \
    }                                                    \
})

extern bool KILKET_DEBUG;
extern bool KILKET_VERBOSE;
extern bool KILKET_QUIET;

#define FW_LOG(msg) if (KILKET_DEBUG) std::cout << msg << std::endl
#define FW_VERBOSE(msg) if (KILKET_VERBOSE) std::cout << msg << std::endl
#define LOG_BUILD_OUTPUT(msg) if (!KILKET_QUIET) std::cout << msg
