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
        if ( val > 0x7fff )
            pOutput[i].left = 0x7fff;
        else if ( val < (short)0x8000 )
            pOutput[i].left = (short)0x8000;
        else
            pOutput[i].left = val;

        val = (pInput[i].right * nVolume) >> 8;
        if ( val > 0x7fff )
            pOutput[i].right = 0x7fff;
        else if ( val < (short)0x8000 )
            pOutput[i].right = (short)0x8000;
        else
            pOutput[i].right = val;
    }
}
