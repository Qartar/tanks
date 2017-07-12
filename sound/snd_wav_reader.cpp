/*=========================================================
Name    :   snd_wav_reader.cpp
Date    :   04/07/2006
=========================================================*/

#include "snd_main.h"
#include "snd_wav_source.h"

/*=========================================================
=========================================================*/

#define RIFF_ID     MAKEID('R','I','F','F')
#define WAVE_ID     MAKEID('W','A','V','E')

riffChunk_c::riffChunk_c (char const *szFilename)
{
    int     name;

    m_pos = 0;

    file::open( &m_riff, szFilename, "rb" );
    if ( !m_riff )
    {
        m_chunkName = NULL;
        m_chunkSize = NULL;
        return;
    }

    name = readInt( );
    if ( name != RIFF_ID )
    {
        m_chunkName = NULL;
        m_chunkSize = NULL;
        return;
    }
    else
    {
        m_size = readInt( );
        m_name = readInt( );

        m_start = m_pos;

        if ( m_name != WAVE_ID )
        {
            m_chunkName = NULL;
            m_chunkSize = NULL;
        }
    }

    chunkSet( );
}

riffChunk_c::riffChunk_c (byte* pChunkData, int nChunkSize)
{
    int     name;

    m_pos = 0;

    m_riff = NULL;
    m_riffData = pChunkData;

    if ( !m_riffData )
    {
        m_chunkName = NULL;
        m_chunkSize = NULL;
        return;
    }

    name = readInt( );
    if ( name != RIFF_ID )
    {
        m_chunkName = NULL;
        m_chunkSize = NULL;
        return;
    }
    else
    {
        m_size = readInt( );
        m_name = readInt( );

        m_start = m_pos;

        if ( m_name != WAVE_ID )
        {
            m_chunkName = NULL;
            m_chunkSize = NULL;
        }
    }

    chunkSet( );
}

riffChunk_c::riffChunk_c (riffChunk_c &Outer)
{
    m_size = Outer.m_size;
    m_name = Outer.m_name;
    m_start = Outer.m_start;

    m_pos = Outer.m_chunkStart + 8;

    chunkSet( );
}

void riffChunk_c::chunkClose ()
{
    if ( m_riff ) {
        file::close( m_riff );
        m_riff = NULL;
    }
}

/*=========================================================
=========================================================*/

int riffChunk_c::m_read (void *out, int len)
{
    if ( m_riff ) {

        int read = fread( out, 1, len, m_riff );
        m_pos += read;
        return read;

    } else if ( m_riffData ) {

        memcpy( out, m_riffData + m_pos, len );
        m_pos += len;
        return len;

    } else {
        return 0;
    }
}

int riffChunk_c::readChunk (byte *pOutput)
{
    return m_read( pOutput, m_chunkSize );
}

int riffChunk_c::readData (byte *pOutput, int nLength)
{
    return m_read( pOutput, nLength );
}

int riffChunk_c::readInt ()
{
    int     i;

    m_read( &i, 4 );

    return data::littlelong( i );
}

/*=========================================================
=========================================================*/

int riffChunk_c::getPos ()
{
    return m_pos;
}

int riffChunk_c::setPos (int pos)
{
    m_pos = pos;
    if ( m_riff ) {
        fseek( m_riff, pos, SEEK_SET );
    }
    return m_pos;
}

/*=========================================================
=========================================================*/

unsigned int riffChunk_c::name ()
{
    return m_chunkName;
}

int riffChunk_c::getSize ()
{
    return m_chunkSize;
}

/*=========================================================
=========================================================*/

void riffChunk_c::chunkSet ()
{
    if ( m_pos < 0 || m_pos > m_start + m_size )
    {
        m_chunkName = 0;
        m_chunkSize = 0;
        return;
    }
    m_chunkStart = m_pos;

    m_chunkName = readInt( );
    m_chunkSize = readInt( );
}

/*=========================================================
=========================================================*/

bool riffChunk_c::chunkNext ()
{
    int     nextChunk = m_chunkStart + m_chunkSize + 8;

    nextChunk += m_chunkSize & 1;

    if ( nextChunk > m_start + m_size )
    {
        m_chunkSize = -1;
        m_chunkName = 0;
        return false;
    }

    setPos( nextChunk );

    chunkSet( );

    return true;
}
