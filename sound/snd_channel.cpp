/*=========================================================
Name    :   snd_channel.cpp
Date    :   04/02/2006
=========================================================*/

#include "snd_main.h"

/*=========================================================
=========================================================*/

sound::channel *cSound::m_allocChan (bool bReserve)
{
    int         i;

    if ( !pAudioDevice )
        return NULL;

    for ( i=0 ; i<MAX_CHANNELS ; i++ )
    {
        if ( m_Channels[i].playing() )
            continue;

        if ( m_Channels[i].isReserved() )
            continue;

        if ( bReserve )
            m_Channels[i].setReserved( true );
        return (sound::channel *)&m_Channels[i];
    }
    return NULL;
}

void cSound::free_channel (sound::channel *pChan)
{
    cSoundChannel   *pChannel = (cSoundChannel *)pChan;

    pChannel->stop( );
    pChannel->setReserved( false );
}

void cSound::m_mixChannels (paintbuffer_t *pBuffer, int nSamples)
{
    int         i;

    for ( i=0 ; i<MAX_CHANNELS ; i++ )
        if ( m_Channels[i].playing( ) )
            m_Channels[i].mixChannel( pBuffer, nSamples );
}

/*=========================================================
=========================================================*/

int cSoundChannel::play (sound::asset asset, bool bLooping)
{
    if ( asset == sound::asset::invalid )
        return ERROR_FAIL;

    m_pSound = gSound->getSound( static_cast<int>(asset) - 1 );

    if ( !m_pSound )
    {
        pMain->message( "could not play sound %i: does not exist\n", static_cast<int>(asset) - 1 );
        return ERROR_FAIL;
    }

    m_nSamplePos = 0;
    m_bPlaying = true;
    m_bLooping = bLooping;

    return ERROR_NONE;
}

int cSoundChannel::play (sound::asset asset)
{
    return play( asset, false );
}

int cSoundChannel::loop (sound::asset asset)
{
    return play( asset, true );
}

void cSoundChannel::stop ()
{
    m_bPlaying = false;
    m_bLooping = false;
}

/*=========================================================
=========================================================*/

void cSoundChannel::mixChannel (paintbuffer_t *pBuffer, int nSamples)
{
    int     nRemaining = nSamples;
    int     volume = (int)(m_flVolume * pBuffer->nVolume * 255) >> 8;

    soundFormat_t   *format = m_pSound->getFormat( );

    float   rate = m_flFrequency * (float)format->frequency / (float)pBuffer->nFrequency;

    if ( format->channels == 1 && format->bitwidth == 16 )
        m_mixMono16( pBuffer->pData, rate, volume, nRemaining );
    if ( format->channels == 2 && format->bitwidth == 16 )
        m_mixStereo16( pBuffer->pData, rate, volume, nRemaining );
}

#define SAMPLE(a,b,c)   ( (float)(a[b]*(1-c))+(float)(a[b+1]*(c)) )

void cSoundChannel::m_mixMono16 (void *pBuffer, float flRate, int nVolume, int nSamples)
{
    samplepair_t    *output = (samplepair_t *)pBuffer;
    short           *input = (short *)gSound->getChannelBuffer( )->pData;

    int         nSamplesRead;
    float       nSamplePos;

    int     sampleIndex;
    float   sampleFrac;

    int     spatial_vol[2];

    m_SpatializeStereo( nVolume, spatial_vol );

    nSamplesRead = m_pSound->getSamples( (byte *)input, nSamples*flRate, m_nSamplePos, m_bLooping );
    nSamplePos = m_nSamplePos - floor( m_nSamplePos );

    for ( int i=0 ; nSamplePos<nSamplesRead ; i++ )
    {
        sampleIndex = floor( nSamplePos );
        sampleFrac = nSamplePos - sampleIndex;

        output[i].left += (int)(spatial_vol[0] * SAMPLE(input,sampleIndex,sampleFrac)) >> 8;
        output[i].right += (int)(spatial_vol[1] * SAMPLE(input,sampleIndex,sampleFrac)) >> 8;

        m_nSamplePos += flRate;
        nSamplePos += flRate;
    }

    if ( nSamplesRead < (int)nSamples*flRate )
        m_bPlaying = false;

    m_nSamplePos = m_pSound->getLoopPosition( m_nSamplePos );
}

#undef SAMPLE
#define SAMPLE(a,b,c,d) ( (float)(a[c].b*(1-d))+(float)(a[c+1].b*(d)) )

void cSoundChannel::m_mixStereo16 (void *pBuffer, float flRate, int nVolume, int nSamples)
{
    samplepair_t    *output = (samplepair_t *)pBuffer;
    stereo16_t      *input = (stereo16_t *)gSound->getChannelBuffer( )->pData;

    int         nSamplesRead;
    float       nSamplePos;

    int     sampleIndex;
    float   sampleFrac;

    int     spatial_vol[2];

    m_SpatializeStereo( nVolume, spatial_vol );

    nSamplesRead = m_pSound->getSamples( (byte *)input, nSamples*flRate, m_nSamplePos, m_bLooping );
    nSamplePos = m_nSamplePos - floor( m_nSamplePos );

    for ( int i=0 ; nSamplePos<nSamplesRead ; i++ )
    {
        sampleIndex = floor( nSamplePos );
        sampleFrac = nSamplePos - sampleIndex;

        output[i].left += (int)(spatial_vol[0] * SAMPLE(input,left,sampleIndex,sampleFrac)) >> 8;
        output[i].right += (int)(spatial_vol[1] * SAMPLE(input,right,sampleIndex,sampleFrac)) >> 8;

        m_nSamplePos += flRate;
        nSamplePos += flRate;
    }

    if ( nSamplesRead < (int)nSamples*flRate )
        m_bPlaying = false;

    m_nSamplePos = m_pSound->getLoopPosition( m_nSamplePos );
}

#undef SAMPLE

/*=========================================================
=========================================================*/

#define ATTN_LEN        1000.0f

void cSoundChannel::m_SpatializeMono (int in, int *out)
{
    if ( m_flAttenuation == ATTN_STATIC )
        out[0] = in;
    else
    {
        vec3    dir = m_vOrigin - gSound->_origin;
        dir.normalize_self( );

        float   attn = clamp( powf(ATTN_LEN / dir.length_sqr( ), m_flAttenuation), 0.0f, 1.0f );

        out[0] = in * attn;
    }
}

void cSoundChannel::m_SpatializeStereo (int in, int out[])
{
    vec3 dir = m_vOrigin - gSound->_origin;
    float dist = dir.normalize_length();
    float dp = dir.dot(gSound->_axis[1]);

    if ( m_flAttenuation == ATTN_STATIC )
    {
        out[0] = in * 0.5f * (1.0f - dp);
        out[1] = in * 0.5f * (1.0f + dp);
    }
    else
    {
        float   attn = clamp( powf(ATTN_LEN / dist, m_flAttenuation), 0.0f, 1.0f );

        out[0] = in * 0.5f * (1 - dp) * attn;
        out[1] = in * 0.5f * (1 + dp) * attn;
    }
}
