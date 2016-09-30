/*=========================================================
Name	:	snd_wav_source.cpp
Date	:	04/07/2006
=========================================================*/

#include <windows.h>

#include "snd_main.h"
#include "snd_wav_source.h"
#include "snd_wav_cache.h"
#include "snd_wav_stream.h"

/*=========================================================
=========================================================*/

cSoundSource *cSoundSource::createSound (char *szFilename)
{
	int			len = strlen(szFilename);
	int			filelen;

	cSoundSource	*pSource = NULL;

	if ( strcmp( szFilename+len-4, ".wav" ) == 0 )
	{
		filelen = file::length( szFilename );

		if ( filelen > STREAM_THRESHOLD )
			pSource = (cSoundSource *)new cSoundWaveStream;
		else
			pSource = (cSoundSource *)new cSoundWaveCache;
	}
	else
		pMain->Message( "unknown sound format: %s\n", szFilename );

	if ( pSource && pSource->Load( szFilename ) == ERROR_NONE )
		return pSource;
	else if ( pSource )
		delete pSource;
	
	return NULL;
}

void cSoundSource::destroySound (cSoundSource *pSound)
{
	if ( pSound )
	{
		pSound->Unload( );
		delete pSound;
	}
}

/*=========================================================
=========================================================*/

void cSoundWaveSource::parseChunk (riffChunk_t &chunk)
{
	switch ( chunk.getName( ) )
	{
	case CHUNK_FMT:
		parseFormat( chunk );
		break;

	case CHUNK_CUE:
		parseCue( chunk );
		break;

	case CHUNK_DATA:
		parseData( chunk );
		break;

	default:
		break;
	}
}

/*=========================================================
=========================================================*/

void cSoundWaveSource::parseFormat (riffChunk_t &chunk)
{
	WAVEFORMATEX		wfx;

	chunk.readData( (byte *)&wfx, chunk.getSize( ) );

	m_format.format = data::littleshort( wfx.wFormatTag );
	m_format.channels = data::littleshort( wfx.nChannels );
	m_format.bitwidth = data::littleshort( wfx.wBitsPerSample );
	m_format.frequency = data::littlelong( wfx.nSamplesPerSec );
}

/*=========================================================
=========================================================*/

void cSoundWaveSource::parseCue (riffChunk_t &chunk)
{
	struct cuePoint_s
	{
		int		cueID;
		int		cuePos;
		int		chunkID;
		int		chunkStart;
		int		blockStart;
		int		sampleOffset;
	} cue_point;

	int	cue_count = chunk.readInt( );

	chunk.readData( (byte *)&cue_point, sizeof(cue_point) );
	m_loopStart = cue_point.sampleOffset;

	// dont care about the rest
}

/*=========================================================
=========================================================*/

float cSoundWaveSource::getLoopPosition (float flPosition)
{
	while ( flPosition > m_numSamples )
		flPosition -= m_numSamples;

	return flPosition;
}