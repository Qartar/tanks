/*=========================================================
Name    :   snd_wav_stream.h
Date    :   04/07/2006
=========================================================*/

#pragma once

#include "snd_wav_source.h"

/*=========================================================
=========================================================*/

class cSoundWaveStream : public cSoundWaveSource
{
public:
    virtual int             getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping);

    virtual result  Load (char const *szFilename);
    virtual void    Unload ();

private:
    virtual void    parseData   (riffChunk_t &chunk);

    result          readData (byte *pOutput, int nStart, int nBytes);

    int     m_dataOffset;   // data chunk
    int     m_dataSize;     // in bytes

    riffChunk_t     *m_reader;
};
