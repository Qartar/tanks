/*=========================================================
Name    :   snd_channel.cpp
Date    :   04/02/2006
=========================================================*/

#include "snd_main.h"

/*=========================================================
=========================================================*/

void cSound::mixStereo16 (samplepair_t *pInput, stereo16_t *pOutput, int nSamples, int nVolume)
{
    int         i, val;

    for ( i=0 ; i<nSamples ; i++ )
    {
        val = (pInput[i].left * nVolume) >> 8;
        pOutput[i].left = clamp<int>(val, INT16_MIN, INT16_MAX);

        val = (pInput[i].right * nVolume) >> 8;
        pOutput[i].right = clamp<int>(val, INT16_MIN, INT16_MAX);
    }
}
