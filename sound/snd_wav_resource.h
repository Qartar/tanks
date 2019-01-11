// snd_wav_resource.h
//

#pragma once

#include "snd_wav_cache.h"

//------------------------------------------------------------------------------
class cSoundWaveResource : public cSoundWaveCache
{
public:
    virtual result load(char const* filename) override;
};
