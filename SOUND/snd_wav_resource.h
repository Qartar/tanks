/*=========================================================
Name    :   snd_wav_resource.h
Date    :   10/14/2016
=========================================================*/

#include "snd_wav_cache.h"

/*=========================================================
=========================================================*/

class cSoundWaveResource : public cSoundWaveCache
{
public:
    virtual int     Load (char *szFilename);

protected:
    HANDLE m_hResource;
    LPVOID m_lpvData;
};
