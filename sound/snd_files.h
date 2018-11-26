/*=========================================================
Name    :   snd_files.h
Date    :   04/07/2006
=========================================================*/

#pragma once

#include "cm_sound.h"

/*=========================================================
=========================================================*/

typedef struct soundFormat_s
{
    int     format;
    int     channels;
    int     bitwidth;
    int     frequency;
} soundFormat_t;

class cSoundSource
{
public:
    static cSoundSource *createSound (char const *szFilename);
    static void         destroySound (cSoundSource *pSound);

    virtual std::size_t getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping) = 0;
    virtual soundFormat_t   *getFormat () = 0;
    virtual char            *getFilename () = 0;
    virtual float           getLoopPosition (float flPosition) = 0;

private:
    virtual result  Load (char const *szFilename) = 0;
    virtual void    Unload () = 0;
};

/*=========================================================
=========================================================*/
