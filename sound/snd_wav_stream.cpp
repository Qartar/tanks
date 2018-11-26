/*=========================================================
Name    :   snd_wav_stream.cpp
Date    :   04/07/2006
=========================================================*/

#include "snd_main.h"
#include "snd_wav_stream.h"

/*=========================================================
=========================================================*/

result cSoundWaveStream::Load (char const *szFilename)
{
    m_reader = new riffChunk_c( szFilename );

    while ( m_reader->name( ) )
    {
        parseChunk( *m_reader );
        m_reader->chunkNext( );
    }

    return (m_numSamples > 0 ? result::success : result::failure);
}

void cSoundWaveStream::Unload ()
{
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

std::size_t cSoundWaveStream::getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping)
{
    std::size_t nRemaining;
    std::size_t nBytes, nStart;

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

result cSoundWaveStream::readData (byte *pOutput, std::size_t nStart, std::size_t nBytes)
{
    m_reader->setPos( m_dataOffset + nStart );
    m_reader->readData( pOutput, nBytes );

    std::size_t fin = nBytes / (m_format.bitwidth / 8);

    for (std::size_t ii = 0; ii < fin; ++ii)
    {
        if ( m_format.bitwidth == 8 )
        {
            int sample = (int)((unsigned char)(pOutput[ii]) - 128);
            ((signed char *)pOutput)[ii] = sample;
        }
    }

    return result::success;
}
