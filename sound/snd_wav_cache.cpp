// snd_wav_cache.cpp
//

#include "snd_main.h"
#include "snd_wav_cache.h"

//------------------------------------------------------------------------------
result cSoundWaveCache::load(string::view filename)
{
    chunk_file reader(filename);

    while (reader.id()) {
        parse_chunk(reader);
        reader.next();
    }

    return (_num_samples > 0 ? result::success : result::failure);
}

//------------------------------------------------------------------------------
void cSoundWaveCache::free()
{
    delete[] _data;
    _data = nullptr;
    _data_size = 0;
}

//------------------------------------------------------------------------------
bool cSoundWaveCache::parse_data(chunk_file& chunk)
{
    _data_size = chunk.size();
    _data = new byte[_data_size];

    _num_samples = _data_size / (_format.channels * _format.bitwidth / 8);

    //
    //  read
    //

    chunk.read(_data, _data_size);

    //
    //  convert
    //

    for (std::size_t ii = 0; ii < _num_samples; ++ii) {
        if (_format.bitwidth == 8) {
            int sample = (int)((unsigned char)(_data[ii]) - 128);
            ((signed char *)_data)[ii] = narrow_cast<char>(sample);
        }
    }

    return true;
}

//------------------------------------------------------------------------------
std::size_t cSoundWaveCache::get_samples(byte* samples, int num_samples, int sample_offset, bool looping)
{
    std::size_t sample_size = _format.channels * _format.bitwidth / 8;

    std::size_t num_bytes = num_samples * sample_size;
    std::size_t offset_bytes = sample_offset * sample_size;

    std::size_t bytes_remaining = num_bytes;
    std::size_t bytes_read = 0;

    if (offset_bytes + num_bytes > _data_size) {
        num_bytes = _data_size - offset_bytes;
    }

    memcpy(samples, _data + offset_bytes, num_bytes);

    bytes_remaining -= num_bytes;
    bytes_read += num_bytes;

    while (bytes_remaining && looping) {
        num_bytes = bytes_remaining;

        if (_loop_start > 0) {
            std::size_t loop_bytes = _loop_start * sample_size;

            if (loop_bytes + num_bytes > _data_size) {
                num_bytes = _data_size - loop_bytes;
            }

            memcpy(samples + bytes_read, _data + loop_bytes, num_bytes);
            bytes_remaining -= num_bytes;
            bytes_read += num_bytes;
        } else {
            if (num_bytes > _data_size) {
                num_bytes = _data_size;
            }

            memcpy(samples + bytes_read, _data, num_bytes);
            bytes_remaining -= num_bytes;
            bytes_read += num_bytes;
        }
    }

    return bytes_read / sample_size;
}
