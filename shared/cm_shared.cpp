// cm_shared.cpp
//

#include "cm_shared.h"

//------------------------------------------------------------------------------
char const* va (char const *fmt, ...)
{
    constexpr int buffer_size = 16384;
    static thread_local int  index = 0;
    static thread_local char string[buffer_size];

    va_list     ap;
    char        *buf;

    int offset = index & (buffer_size-1);
    buf = string + offset;

    va_start(ap, fmt);
    int len = vsnprintf(buf, buffer_size - offset, fmt, ap);
    va_end(ap);

    // check if a wrap is necessary
    if (len >= buffer_size - offset && len < buffer_size) {
        buf = string;
        index = 0;

        va_start(ap, fmt);
        len = vsnprintf(buf, buffer_size, fmt, ap);
        va_end(ap);
    }

    // vsnprintf returns a negative value if an error occurs
    index += max(0, len + 1);
    return buf;
}

////////////////////////////////////////////////////////////////////////////////
namespace detail {

log* log::_singleton = nullptr;

//------------------------------------------------------------------------------
void log::set(log* logger)
{
    _singleton = logger;
}

//------------------------------------------------------------------------------
void log::message(char const* fmt, ...)
{
    if (_singleton) {
        constexpr int buffer_size = 8192;
        char buffer[buffer_size];

        va_list ap;

        va_start(ap, fmt);
        vsnprintf(buffer, buffer_size, fmt, ap);
        va_end(ap);

        _singleton->print(level::message, buffer);
    }
}

//------------------------------------------------------------------------------
void log::warning(char const* fmt, ...)
{
    if (_singleton) {
        constexpr int buffer_size = 8192;
        char buffer[buffer_size];

        va_list ap;

        va_start(ap, fmt);
        vsnprintf(buffer, buffer_size, fmt, ap);
        va_end(ap);

        _singleton->print(level::warning, buffer);
    }
}

//------------------------------------------------------------------------------
void log::error(char const* fmt, ...)
{
    if (_singleton) {
        constexpr int buffer_size = 8192;
        char buffer[buffer_size];

        va_list ap;

        va_start(ap, fmt);
        vsnprintf(buffer, buffer_size, fmt, ap);
        va_end(ap);

        _singleton->print(level::error, buffer);
    }
}

} // namespace detail
