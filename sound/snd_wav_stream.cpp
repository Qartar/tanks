// snd_wav_stream.cpp
//

#include "snd_main.h"
#include "snd_wav_stream.h"

//------------------------------------------------------------------------------
result cSoundWaveStream::load(char const* filename)
{
    _reader = std::make_unique<chunk_file>(filename);

    while (_reader->id()) {
        parse_chunk(*_reader);
        _reader->next();
    }

    return (_num_samples > 0 ? result::success : result::failure);
}

//------------------------------------------------------------------------------
void cSoundWaveStream::free ()
{
    _reader = nullptr;
}

//------------------------------------------------------------------------------
bool cSoundWaveStream::parse_data(chunk_file& chunk)
{
    _data_offset = chunk.tell();
    _data_size = chunk.size();

    _num_samples = _data_size / (_format.channels * _format.bitwidth / 8);

    return true;
}

//------------------------------------------------------------------------------
std::size_t cSoundWaveStream::get_samples(byte* samples, int num_samples, int sample_offset, bool looping)
{
    std::size_t sample_size = _format.channels * _format.bitwidth / 8;

    std::size_t num_bytes = num_samples * sample_size;
    std::size_t offset_bytes = sample_offset * sample_size;

    std::size_t bytes_remaining = num_bytes;

    if (num_bytes + offset_bytes > _data_size) {
        num_bytes = _data_size - offset_bytes;
    }

    read(samples, offset_bytes, num_bytes);
    bytes_remaining -= num_bytes;

    if (bytes_remaining && looping) {
        if (_loop_start) {
            std::size_t loop_bytes = _loop_start * sample_size;
            read(samples + num_bytes, loop_bytes, bytes_remaining);
        } else {
            read(samples + num_bytes, 0, bytes_remaining);
        }

        return (num_bytes + bytes_remaining) / sample_size;
    }

    return num_bytes / sample_size;
}

//------------------------------------------------------------------------------
result cSoundWaveStream::read(byte *data, std::size_t start, std::size_t size)
{
    _reader->seek(_data_offset + start);
    _reader->read(data, size);

    std::size_t fin = size / (_format.bitwidth / 8);

    for (std::size_t ii = 0; ii < fin; ++ii) {
        if (_format.bitwidth == 8) {
            int sample = (int)((unsigned char)(data[ii]) - 128);
            ((signed char *)data)[ii] = sample;
        }
    }

    return result::success;
}
