/*=========================================================
Name	:	snd_channel.cpp
Date	:	04/02/2006
=========================================================*/

#include "snd_main.h"

/*=========================================================
=========================================================*/

sndchan_t *cSound::m_allocChan (bool bReserve)
{
	int			i;

	if ( !pAudioDevice )
		return NULL;

	for ( i=0 ; i<MAX_CHANNELS ; i++ )
	{
		if ( m_Channels[i].isPlaying() )
			continue;

		if ( m_Channels[i].isReserved() )
			continue;

		if ( bReserve )
			m_Channels[i].setReserved( true );
		return (sndchan_t *)&m_Channels[i];
	}
	return NULL;
}

void cSound::freeChan (sndchan_t *pChan)
{
	cSoundChannel	*pChannel = (cSoundChannel *)pChan;

	pChannel->stopSound( );
	pChannel->setReserved( false );
}

void cSound::m_mixChannels (paintbuffer_t *pBuffer, int nSamples)
{
	int			i;

	for ( i=0 ; i<MAX_CHANNELS ; i++ )
		if ( m_Channels[i].isPlaying( ) )
			m_Channels[i].mixChannel( pBuffer, nSamples );
}

/*=========================================================
=========================================================*/

int cSoundChannel::playSound (int nSound, bool bLooping)
{
	if ( nSound < 0 )
		return ERROR_FAIL;

	m_pSound = gSound->getSound( nSound );

	if ( !m_pSound )
	{
		pMain->Message( "could not play sound %i: does not exist\n", nSound );
		return ERROR_FAIL;
	}

	m_nSamplePos = 0;
	m_bPlaying = true;
	m_bLooping = bLooping;

	return ERROR_NONE;
}

int cSoundChannel::playSound (int nSound)
{
	return playSound( nSound, false );
}

int cSoundChannel::playLoop (int nSound)
{
	return playSound( nSound, true );
}

void cSoundChannel::stopSound ()
{
	m_bPlaying = false;
	m_bLooping = false;
}

/*=========================================================
=========================================================*/

void cSoundChannel::mixChannel (paintbuffer_t *pBuffer, int nSamples)
{
	int		nRemaining = nSamples;
	int		volume = (int)(m_flVolume * pBuffer->nVolume * 255) >> 8;

	soundFormat_t	*format = m_pSound->getFormat( );

	float	rate = m_flFrequency * (float)format->frequency / (float)pBuffer->nFrequency;

	if ( format->channels == 1 && format->bitwidth == 16 )
		m_mixMono16( pBuffer->pData, rate, volume, nRemaining );
	if ( format->channels == 2 && format->bitwidth == 16 )
		m_mixStereo16( pBuffer->pData, rate, volume, nRemaining );
}

#define SAMPLE(a,b,c)	( (float)(a[b]*(1-c))+(float)(a[b+1]*(c)) )

void cSoundChannel::m_mixMono16 (void *pBuffer, float flRate, int nVolume, int nSamples)
{
	samplepair_t	*output = (samplepair_t *)pBuffer;
	short			*input = (short *)gSound->getChannelBuffer( )->pData;

	int			nSamplesRead;
	float		nSamplePos;

	int		sampleIndex;
	float	sampleFrac;

	int		spatial_vol;

	m_SpatializeMono( nVolume, &spatial_vol );

	nSamplesRead = m_pSound->getSamples( (byte *)input, nSamples*flRate, m_nSamplePos, m_bLooping );
	nSamplePos = m_nSamplePos - floor( m_nSamplePos );

	for ( int i=0 ; nSamplePos<nSamplesRead ; i++ )
	{
		sampleIndex = floor( nSamplePos );
		sampleFrac = nSamplePos - sampleIndex;

		output[i].left += (int)(spatial_vol * SAMPLE(input,sampleIndex,sampleFrac)) >> 8;
		output[i].right += (int)(spatial_vol * SAMPLE(input,sampleIndex,sampleFrac)) >> 8;

		m_nSamplePos += flRate;
		nSamplePos += flRate;
	}

	if ( nSamplesRead < (int)nSamples*flRate )
		m_bPlaying = false;

	m_nSamplePos = m_pSound->getLoopPosition( m_nSamplePos );
}

#undef SAMPLE
#define SAMPLE(a,b,c,d)	( (float)(a[c].b*(1-d))+(float)(a[c+1].b*(d)) )

void cSoundChannel::m_mixStereo16 (void *pBuffer, float flRate, int nVolume, int nSamples)
{
	samplepair_t	*output = (samplepair_t *)pBuffer;
	stereo16_t		*input = (stereo16_t *)gSound->getChannelBuffer( )->pData;

	int			nSamplesRead;
	float		nSamplePos;

	int		sampleIndex;
	float	sampleFrac;

	int		spatial_vol[2];

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

#define ATTN_LEN		1000.0f

void cSoundChannel::m_SpatializeMono (int in, int *out)
{
	if ( m_flAttenuation == ATTN_STATIC )
		out[0] = in;
	else
	{
		vec3	dir = m_vOrigin - gSound->vOrigin;
		dir.normalize( );

		float	attn = clamp( powf(ATTN_LEN / dir.lengthsq( ), m_flAttenuation), 0, 1 );

		out[0] = in * attn;
	}
}

void cSoundChannel::m_SpatializeStereo (int in, int out[])
{
	if ( m_flAttenuation == ATTN_STATIC )
	{
		out[0] = in;
		out[1] = in;
	}
	else
	{
		vec3	vDir;
		float	len;
		float	dotRight;

		vDir = m_vOrigin - gSound->vOrigin;
		len = vDir.length( );
		vDir.normalize( );

		dotRight = vDir.dot( gSound->vRight );

		float	attn = clamp( powf(ATTN_LEN / len, m_flAttenuation), 0, 1 );

		out[0] = in * (0.5f * (1 + dotRight) * attn);
		out[1] = in * (0.5f * (1 - dotRight) * attn);
	}
}