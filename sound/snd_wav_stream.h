// snd_wav_stream.h
//

#pragma once

#include "snd_wav_source.h"

//------------------------------------------------------------------------------
class cSoundWaveStream : public cSoundWaveSource
{
public:
    virtual ~cSoundWaveStream() { free(); }

    virtual std::size_t get_samples(byte* samples, int num_samples, int sample_offset, bool looping) override;

    virtual result load(string::view filename) override;
    virtual void free() override;

private:
    virtual bool parse_data(chunk_file& chunk) override;

    result read(byte* data, std::size_t start, std::size_t size);

    std::size_t _data_offset; // data chunk
    std::size_t _data_size; // in bytes

    std::unique_ptr<chunk_file> _reader;
};
