/*=========================================================
Name    :   snd_wav_stream.cpp
Date    :   04/07/2006
=========================================================*/

#include "snd_main.h"
#include "snd_wav_stream.h"

/*=========================================================
=========================================================*/

int cSoundWaveStream::Load (char const *szFilename)
{
    m_reader = new riffChunk_c( szFilename );

    while ( m_reader->getName( ) )
    {
        parseChunk( *m_reader );
        m_reader->chunkNext( );
    }

    return (m_numSamples > 0 ? ERROR_NONE : ERROR_FAIL);
}

void cSoundWaveStream::Unload ()
{
    m_reader->chunkClose( );
    delete m_reader;
}

/*=========================================================
=========================================================*/

void cSoundWaveStream::parseData (riffChunk_t &chunk)
{
    m_dataOffset = chunk.getPos( );
    m_dataSize = chunk.getSize( );

    m_numSamples = m_dataSize / (m_format.channels * m_format.bitwidth / 8);
}

/*=========================================================
=========================================================*/

int cSoundWaveStream::getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping)
{
    int     nRemaining;
    int     nBytes, nStart;

    int     nSampleSize = m_format.channels * m_format.bitwidth / 8;

    nBytes = nSamples * nSampleSize;
    nStart = nOffset * nSampleSize;

    nRemaining = nBytes;

    if ( nBytes + nStart > m_dataSize )
        nBytes = m_dataSize - nStart;

    readData( pOutput, nStart, nBytes );
    nRemaining -= nBytes;

    if ( nRemaining && bLooping )
    {
        if ( m_loopStart )
        {
            int loopBytes = m_loopStart * nSampleSize;
            readData( pOutput+nBytes, loopBytes, nRemaining );
        }
        else
            readData( pOutput+nBytes, 0, nRemaining );

        return (nBytes + nRemaining) / nSampleSize;
    }

    return nBytes / nSampleSize;
}

/*=========================================================
=========================================================*/

int cSoundWaveStream::readData (byte *pOutput, int nStart, int nBytes)
{
    int         i, fin, sample;

    m_reader->setPos( m_dataOffset + nStart );
    m_reader->readData( pOutput, nBytes );

    fin = nBytes / (m_format.bitwidth / 8);

    for ( i=0 ; i<fin ; i++ )
    {
        if ( m_format.bitwidth == 16 )
        {
            sample = data::littleshort( ((short *)pOutput)[i] );
            ((short *)pOutput)[i] = sample;
        }
        else
        {
            sample = (int)((unsigned char)(pOutput[i]) - 128);
            ((signed char *)pOutput)[i] = sample;
        }
    }

    return ERROR_NONE;
}
