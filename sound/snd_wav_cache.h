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
    virtual int             getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping);

    virtual result  Load (char const *szFilename);
    virtual void    Unload ();

protected:
    virtual void    parseData   (riffChunk_t &chunk);

    byte    *m_dataCache;   // data chunk
    int     m_cacheSize;    // in bytes
};
