/*=========================================================
Name    :   snd_wav_cache.h
Date    :   04/07/2006
=========================================================*/

#pragma once

#include "snd_wav_source.h"

/*=========================================================
=========================================================*/

class cSoundWaveCache : public cSoundWaveSource
{
public:
    virtual std::size_t getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping);

    virtual result  Load (char const *szFilename);
    virtual void    Unload ();

protected:
    virtual void    parseData   (riffChunk_t &chunk);

    byte    *m_dataCache;   // data chunk
    std::size_t m_cacheSize;    // in bytes
};
