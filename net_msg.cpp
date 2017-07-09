/*
===============================================================================

Name    :   net_msg.cpp

Purpose :   net message formatting

Date    :   04/01/2005

===============================================================================
*/

#include "net_main.h"

#define ANGLE2SHORT(x)  ((int)((x)*(32768.0/360)))
#define SHORT2ANGLE(x)  ((x)*(360.0/32768))

/*
===========================================================

Name    :   initialization / utility functions

===========================================================
*/

void cNetMessage::Init (byte *pData, int nMaxSize)
{
    memset( this, 0, sizeof(netmsg_t) );
    this->pData = pData;
    this->nMaxSize = nMaxSize;

    this->nRemaining = nMaxSize;
    this->nCurSize = 0;
}

void cNetMessage::Clear ()
{
    memset( pData, 0, nCurSize );
    nCurSize = 0;
    nRemaining = nMaxSize;
    bOverflowed = false;
}

void *cNetMessage::Alloc (int nSize)
{
    void    *ret;

    if (nCurSize + nSize > nMaxSize)
    {
        bOverflowed = true;
        return NULL;
    }

    ret = (void *)(pData + nCurSize);
    nCurSize += nSize;
    nRemaining -= nSize;

    return ret;
}

void cNetMessage::Write (void const* pData, int nSize)
{
    void    *buf = Alloc( nSize );

    if ( buf )
        memcpy( buf, pData, nSize );
}

int cNetMessage::Read (void *pOut, int nSize)
{
    if ( nReadCount + nSize > nMaxSize )
        return -1;

    memcpy( pOut, (void *)(pData+nReadCount), nSize );

    nReadCount += nSize;

    return 0;
}

/*
===========================================================

Name    :   writing functions

===========================================================
*/

void cNetMessage::WriteByte (int b)
{
    byte    *buf = (byte *)Alloc( 1 );

    if ( buf )
        buf[0] = b & 0xff;
}

void cNetMessage::WriteShort (int s)
{
    byte    *buf = (byte *)Alloc( 2 );

    if ( buf )
    {
        buf[0] = s & 0xff;
        buf[1] = s>>8;
    }
}

void cNetMessage::WriteLong (int l)
{
    byte    *buf = (byte *)Alloc( 4 );

    if ( buf )
    {
        buf[0] = l & 0xff;
        buf[1] = (l>>8) & 0xff;
        buf[2] = (l>>16) & 0xff;
        buf[3] = (l>>24) & 0xff;
    }
}

void cNetMessage::WriteFloat (float f)
{
    union
    {
        float   f;
        int l;
    } dat;

    dat.f = f;
    dat.l = LITTLE_LONG( dat.l );

    Write( &dat.l, 4 );
}

void cNetMessage::WriteChar (int b)
{
    char    *buf = (char *)Alloc( 1 );

    if ( buf )
        buf[0] = b & 0xff;
}

void cNetMessage::WriteString (char const* sz)
{
    if ( sz )
        Write( sz, strlen(sz)+1 );
    else
        Write( "", 1 );
}

void cNetMessage::WriteAngle (float f)
{
    WriteShort( ANGLE2SHORT(f) );
}

void cNetMessage::WriteVector (vec2 v)
{
    WriteFloat( v.x );
    WriteFloat( v.y );
}

/*
===========================================================

Name    :   reading functions

===========================================================
*/

void cNetMessage::Begin ()
{
    nReadCount = 0;
}

int cNetMessage::ReadByte ()
{
    int b;

    if (nReadCount + 1 > nCurSize)
        b = -1;
    else
        b = (byte )pData[nReadCount];

    nReadCount++;

    return b;
}

int cNetMessage::ReadShort ()
{
    int s;

    if (nReadCount + 2 > nCurSize)
        s = -1;
    else
        s = (short )pData[nReadCount]
        + (pData[nReadCount+1]<<8);

    nReadCount += 2;

    return s;
}

int cNetMessage::ReadLong ()
{
    int l;

    if (nReadCount + 4 > nCurSize)
        l = -1;
    else
        l = (int )pData[nReadCount]
        + (pData[nReadCount+1]<<8)
        + (pData[nReadCount+2]<<16)
        + (pData[nReadCount+3]<<24);

    nReadCount += 4;

    return l;
}

float cNetMessage::ReadFloat ()
{
    union
    {
        byte    b[4];
        float   f;
        int     l;
    } dat;

    if (nReadCount + 4 > nCurSize)
        dat.l = -1;
    else
    {
        dat.b[0] = pData[nReadCount + 0];
        dat.b[1] = pData[nReadCount + 1];
        dat.b[2] = pData[nReadCount + 2];
        dat.b[3] = pData[nReadCount + 3];
    }

    nReadCount += 4;

    dat.l = LITTLE_LONG( dat.l );

    return dat.f;
}

int cNetMessage::ReadChar ()
{
    int b;

    if (nReadCount + 1 > nCurSize)
        b = -1;
    else
        b = (char )pData[nReadCount];

    nReadCount++;

    return b;
}


char *cNetMessage::ReadString ()
{
    static char string[MAX_STRING];
    int         i,c;

    for (i=0 ; i<MAX_STRING-1 ; i++)
    {
        c = ReadByte( );
        if ( c == -1 || c == 0 )
            break;
        string[i] = c;
    }
    string[i] = 0;

    return string;
}


char *cNetMessage::ReadLine ()
{
    static char string[MAX_STRING];
    int         i,c;

    for (i=0 ; i<MAX_STRING-1 ; i++)
    {
        c = ReadByte( );
        if ( c == -1 || c == 0 || c == '\n')
            break;
        string[i] = c;
    }
    string[i] = 0;

    return string;
}

float cNetMessage::ReadAngle ()
{
    int     out;

    out = ReadShort( );

    return ( SHORT2ANGLE(out) );
}

vec2 cNetMessage::ReadVector ()
{
    float   outx, outy;

    if (nReadCount + 8 > nCurSize)
        return vec2(0,0);

    outx = ReadFloat( );
    outy = ReadFloat( );

    return vec2( outx, outy );
}
