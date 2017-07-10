/*=========================================================
Name    :   snd_wav_cache.cpp
Date    :   04/07/2006
=========================================================*/

#include "snd_main.h"
#include "snd_wav_cache.h"

/*=========================================================
=========================================================*/

int cSoundWaveCache::Load (char *szFilename)
{
    riffChunk_t *pReader = new riffChunk_t( szFilename );

    while ( pReader->getName( ) )
    {
        parseChunk( *pReader );
        pReader->chunkNext( );
    }

    pReader->chunkClose( );
    delete pReader;

    return (m_numSamples > 0 ? ERROR_NONE : ERROR_FAIL);
}

void cSoundWaveCache::Unload ()
{
    gSound->heapFree( m_dataCache );
    
    m_dataCache = NULL;
    m_cacheSize = 0;
}

/*=========================================================
=========================================================*/

void cSoundWaveCache::parseData (riffChunk_t &chunk)
{
    int         i, sample;

    m_dataCache = (byte *)gSound->heapAlloc( chunk.getSize( ) );
    m_cacheSize = chunk.getSize( );

    m_numSamples = m_cacheSize / (m_format.channels * m_format.bitwidth / 8);

    //
    //  read
    //

    chunk.readChunk( m_dataCache );

    //
    //  convert
    //

    for ( i=0 ; i<m_numSamples ; i++ )
    {
        if ( m_format.bitwidth == 16 )
        {
            sample = data::littleshort( ((short *)m_dataCache)[i] );
            ((short *)m_dataCache)[i] = sample;
        }
        else
        {
            sample = (int)((unsigned char)(m_dataCache[i]) - 128);
            ((signed char *)m_dataCache)[i] = sample;
        }
    }
}

/*=========================================================
=========================================================*/

int cSoundWaveCache::getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping)
{
    int     nRemaining, nCompleted = 0;
    int     nBytes, nStart;

    int     nSampleSize = m_format.channels * m_format.bitwidth / 8;

    nBytes = nSamples * nSampleSize;
    nStart = nOffset * nSampleSize;

    nRemaining = nBytes;

    if ( nStart + nBytes > m_cacheSize )
        nBytes = m_cacheSize - nStart;

    memcpy( (void *)pOutput, (void *)(m_dataCache+nStart), nBytes );

    nRemaining -= nBytes;
    nCompleted += nBytes;

    while ( nRemaining && bLooping )
    {
        nBytes = nRemaining;

        if ( m_loopStart > 0 )
        {
            int loopBytes = m_loopStart * nSampleSize;

            if ( loopBytes + nBytes > m_cacheSize )
                nBytes = m_cacheSize - loopBytes;

            memcpy( (void *)(pOutput+nCompleted), (void *)(m_dataCache+loopBytes), nBytes );
            nRemaining -= nBytes;
            nCompleted += nBytes;
        }
        else
        {
            if ( nBytes > m_cacheSize )
                nBytes = m_cacheSize;

            memcpy( (void *)(pOutput+nCompleted), (void *)m_dataCache, nBytes );
            nRemaining -= nBytes;
            nCompleted += nBytes;
        }
    }

    return nCompleted / nSampleSize;
}
