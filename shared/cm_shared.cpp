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
