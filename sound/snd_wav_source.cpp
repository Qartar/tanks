/*=========================================================
Name    :   snd_wav_source.cpp
Date    :   04/07/2006
=========================================================*/

#include "snd_main.h"
#include "snd_wav_source.h"
#include "snd_wav_cache.h"
#include "snd_wav_stream.h"
#include "snd_wav_resource.h"

/*=========================================================
=========================================================*/

cSoundSource *cSoundSource::createSound (char const *szFilename)
{
    int         len = strlen(szFilename);
    int         filelen;

    cSoundSource    *pSource = NULL;

    if ( strcmp( szFilename+len-4, ".wav" ) == 0 )
    {
        filelen = file::open( szFilename, file::mode::read ).size();

        if ( filelen == 0 )
            pSource = (cSoundSource *)new cSoundWaveResource;
        else if ( filelen > STREAM_THRESHOLD )
            pSource = (cSoundSource *)new cSoundWaveStream;
        else
            pSource = (cSoundSource *)new cSoundWaveCache;
    }
    else
        pMain->message( "unknown sound format: %s\n", szFilename );

    if ( pSource && succeeded(pSource->Load( szFilename )) )
        return pSource;
    else if ( pSource )
        delete pSource;
    
    return NULL;
}

void cSoundSource::destroySound (cSoundSource *source)
{
    if (source) {
        source->Unload();
        delete source;
    }
}

/*=========================================================
=========================================================*/

void cSoundWaveSource::parseChunk (riffChunk_t &chunk)
{
    switch ( chunk.name( ) )
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
    WAVEFORMATEX        wfx;

    chunk.readData( (byte *)&wfx, chunk.getSize( ) );

    m_format.format = wfx.wFormatTag;
    m_format.channels = wfx.nChannels;
    m_format.bitwidth = wfx.wBitsPerSample;
    m_format.frequency = wfx.nSamplesPerSec;
}

/*=========================================================
=========================================================*/

void cSoundWaveSource::parseCue (riffChunk_t &chunk)
{
    struct cuePoint_s
    {
        int     cueID;
        int     cuePos;
        int     chunkID;
        int     chunkStart;
        int     blockStart;
        int     sampleOffset;
    } cue_point;

    /*int cue_count =*/ chunk.readInt( );

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
