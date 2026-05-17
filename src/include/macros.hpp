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
