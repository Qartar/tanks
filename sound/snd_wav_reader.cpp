// snd_wav_reader.cpp
//

#include "snd_main.h"
#include "snd_wav_source.h"

////////////////////////////////////////////////////////////////////////////////
constexpr int RIFF_ID = make_id('R', 'I', 'F', 'F');
constexpr int WAVE_ID = make_id('W', 'A', 'V', 'E');

//------------------------------------------------------------------------------
chunk_file::chunk_file(string::view filename)
    : _buffer(nullptr)
    , _buffer_size(0)
    , _start(0)
    , _size(0)
    , _id(0)
    , _pos(0)
    , _chunk_id(0)
    , _chunk_size(0)
    , _chunk_start(0)
{
    _stream = file::open(filename, file::mode::read);
    if (!_stream) {
        return;
    }

    if (read_int() == RIFF_ID) {
        _start = _pos;
        _size = read_int();
        _id = read_int();

        if (_id != WAVE_ID) {
            _chunk_id = 0;
            _chunk_size = 0;
        }

        read_chunk();
    }
}

//------------------------------------------------------------------------------
chunk_file::chunk_file(byte const* buffer, std::size_t buffer_size)
    : _buffer(buffer)
    , _buffer_size(buffer_size)
    , _start(0)
    , _size(0)
    , _id(0)
    , _pos(0)
    , _chunk_id(0)
    , _chunk_size(0)
    , _chunk_start(0)
{
    if (!_buffer) {
        return;
    }

    if (read_int() == RIFF_ID) {
        _start = _pos;
        _size = read_int();
        _id = read_int();

        if (_id != WAVE_ID) {
            _chunk_id = 0;
            _chunk_size = 0;
        }

        read_chunk();
    }
}

//------------------------------------------------------------------------------
std::size_t chunk_file::read(byte* buffer, std::size_t buffer_size)
{
    if (_stream) {
        std::size_t read = _stream.read(buffer, buffer_size);
        _pos += read;
        return read;
    } else if (_buffer && _pos + buffer_size <= _buffer_size) {
        memcpy(buffer, _buffer + _pos, buffer_size);
        _pos += buffer_size;
        return buffer_size;
    } else {
        return 0;
    }
}

//------------------------------------------------------------------------------
int chunk_file::read_int()
{
    uint8_t bytes[4];
    read(bytes, 4);

    // return little-endian
    return (bytes[0] <<  0)
         | (bytes[1] <<  8)
         | (bytes[2] << 16)
         | (bytes[3] << 24);
}

//------------------------------------------------------------------------------
std::size_t chunk_file::tell() const
{
    return _pos;
}

//------------------------------------------------------------------------------
std::size_t chunk_file::seek(std::size_t pos)
{
    _pos = pos;
    if (_stream) {
        _stream.seek(pos, file::seek::set);
    }
    return _pos;
}

//------------------------------------------------------------------------------
uint32_t chunk_file::id() const
{
    return _chunk_id;
}

//------------------------------------------------------------------------------
std::size_t chunk_file::size() const
{
    return _chunk_size;
}

//------------------------------------------------------------------------------
bool chunk_file::read_chunk()
{
    if (_pos < 0 || _pos > _start + _size) {
        _chunk_id = 0;
        _chunk_size = 0;
        return false;
    } else {
        _chunk_start = _pos;
        _chunk_id = read_int();
        _chunk_size = read_int();
        return true;
    }
}

//------------------------------------------------------------------------------
bool chunk_file::next()
{
    std::size_t next_chunk = _chunk_start + _chunk_size + 8;

    next_chunk += _chunk_size & 1;

    if (next_chunk > _start + _size) {
        _chunk_size = 0;
        _chunk_id = 0;
        return false;
    }

    seek(next_chunk);
    return read_chunk();
}
