// snd_wav_cache.h
//

#pragma once

#include "snd_wav_source.h"

//------------------------------------------------------------------------------
class cSoundWaveCache : public cSoundWaveSource
{
public:
    virtual std::size_t get_samples(byte* samples, int num_samples, int sample_offset, bool looping) override;

    virtual result load(char const* filename) override;
    virtual void free() override;

protected:
    virtual bool parse_data(chunk_file& chunk) override;

    byte* _data; // data chunk
    std::size_t _data_size; // in bytes
};
