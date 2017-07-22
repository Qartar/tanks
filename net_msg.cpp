// net_msg.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "net_main.h"

////////////////////////////////////////////////////////////////////////////////
namespace network {

//------------------------------------------------------------------------------
message::message(byte* data, std::size_t size)
    : _data(data)
    , _size(size)
    , _bytes_read(0)
    , _bytes_written(0)
    , _bytes_reserved(0)
{}

//------------------------------------------------------------------------------
void message::reset()
{
    _bytes_read = 0;
    _bytes_written = 0;
    _bytes_reserved = 0;
}

//------------------------------------------------------------------------------
void message::rewind() const
{
    _bytes_read = 0;
}

//------------------------------------------------------------------------------
void message::rewind(std::size_t size) const
{
    if (_bytes_read < size) {
        _bytes_read = 0;
    } else {
        _bytes_read -= size;
    }
}

//------------------------------------------------------------------------------
byte* message::write(std::size_t size)
{
    if (_bytes_reserved) {
        return nullptr;
    } else if (_bytes_written + size > _size) {
        return nullptr;
    }

    _bytes_written += size;
    return _data + _bytes_written - size;
}

//------------------------------------------------------------------------------
byte const* message::read(std::size_t size) const
{
    if (_bytes_read + size > _bytes_written) {
        return nullptr;
    }

    _bytes_read += size;
    return _data + _bytes_read - size;
}

//------------------------------------------------------------------------------
byte* message::reserve(std::size_t size)
{
    if (_bytes_written + _bytes_reserved + size > _size) {
        return nullptr;
    }

    _bytes_reserved += size;
    return _data + _bytes_written + _bytes_reserved - size;
}

//------------------------------------------------------------------------------
std::size_t message::commit(std::size_t size)
{
    if (_bytes_reserved < size) {
        _bytes_reserved = 0;
        return 0;
    }

    _bytes_written += size;
    _bytes_reserved = 0;
    return size;
}

//------------------------------------------------------------------------------
std::size_t message::write(byte const* data, std::size_t size)
{
    if (_bytes_written + size > _size) {
        return 0;
    }

    memcpy(_data + _bytes_written, data, size);
    _bytes_written += size;
    return size;
}

//------------------------------------------------------------------------------
std::size_t message::read(byte* data, std::size_t size) const
{
    if (_bytes_read + size > _bytes_written) {
        return 0;
    }

    memcpy(data, _data + _bytes_read, size);
    _bytes_read += size;
    return size;
}

//------------------------------------------------------------------------------
void message::write_byte(int b)
{
    byte* buf = (byte *)write(1);

    if (buf) {
        buf[0] = b & 0xff;
    }
}

//------------------------------------------------------------------------------
void message::write_short(int s)
{
    byte* buf = (byte *)write(2);

    if (buf) {
        buf[0] = s & 0xff;
        buf[1] = s>>8;
    }
}

//------------------------------------------------------------------------------
void message::write_long(int l)
{
    byte* buf = (byte *)write(4);

    if (buf) {
        buf[0] = (l>> 0) & 0xff;
        buf[1] = (l>> 8) & 0xff;
        buf[2] = (l>>16) & 0xff;
        buf[3] = (l>>24) & 0xff;
    }
}

//------------------------------------------------------------------------------
void message::write_float(float f)
{
    union {
        float f;
        int l;
    } dat;

    dat.f = f;
    write_long( dat.l );
}

//------------------------------------------------------------------------------
void message::write_char(int b)
{
    char* buf = (char *)write(1);

    if (buf) {
        buf[0] = b & 0xff;
    }
}

//------------------------------------------------------------------------------
void message::write_string(char const* sz)
{
    if (sz) {
        write((byte const*)sz, strlen(sz) + 1);
    } else {
        write((byte const*)"", 1);
    }
}

//------------------------------------------------------------------------------
void message::write_vector(vec2 v)
{
    write_float(v.x);
    write_float(v.y);
}

//------------------------------------------------------------------------------
int message::read_byte()
{
    byte buf[1];

    if (read(buf, 1) == 1) {
        return buf[0];
    } else {
        return -1;
    }
}

//------------------------------------------------------------------------------
int message::read_short()
{
    byte buf[2];

    if (read(buf, 2) == 2) {
        return short(buf[0] | buf[1] << 8);
    } else {
        return -1;
    }
}

//------------------------------------------------------------------------------
int message::read_long()
{
    byte buf[4];

    if (read(buf, 4) == 4) {
        return short(buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
    } else {
        return -1;
    }
}

//------------------------------------------------------------------------------
float message::read_float()
{
    byte buf[4];

    union {
        float f;
        int l;
    } dat;

    if (read(buf, 4) == 4) {
        dat.l = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
        return dat.f;
    } else {
        return 0.0f;
    }
}

//------------------------------------------------------------------------------
int message::read_char()
{
    byte buf[1];

    if (read(buf, 1) == 1) {
        return char(buf[0]);
    } else {
        return -1;
    }
}

//------------------------------------------------------------------------------
char const* message::read_string()
{
    std::size_t remaining = bytes_remaining();
    char const* str = (char const* )read(remaining);
    std::size_t len = strnlen(str, remaining);

    rewind(remaining - len - 1);
    return (char const*)str;
}

//------------------------------------------------------------------------------
vec2 message::read_vector()
{
    if (_bytes_read + 8 > _bytes_written) {
        return vec2_zero;
    }

    float outx = read_float();
    float outy = read_float();

    return vec2(outx, outy);
}

} // namespace network
