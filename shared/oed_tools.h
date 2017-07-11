/*
===========================================================

Name    :   oed_datatools.h

Purpose :   misc. data manipulation

Modified:   11/03/2006

===========================================================
*/

#pragma once

#include "oed_shared.h"
#include "oed_mem.h"    // [oed_error.h]

#define MAX_ARGC    64

class textutils_c
{
private:
    unsigned int        m_argc;
    char                *m_argv[MAX_ARGC];

public:
    textutils_c () { m_argc = 0; }
    ~textutils_c () { for( unsigned int i=0 ; i<m_argc ; i++ ) mem::free( m_argv[i] ); }

    unsigned int    argc() { return m_argc; }
    char        *argv(unsigned int argc) { if (argc >= m_argc) return "\0"; return m_argv[argc];    }

    int parse (char *string)
    {
        char    *cursor;            // current cursor position
        char    *argptr;            // begining of most recent arg
        char    seqchar;            // char that separates an arg

        unsigned int    i;

        // reset the args

        for( i=0 ; i<m_argc ; i++ )
            mem::free( m_argv[i] );
        m_argc = 0;

        seqchar = K_SPACE;
        cursor = argptr = string;
        do
        {
            if ((*cursor == seqchar) || (*cursor == '\t') || (*cursor == '\n') || (*cursor == '\0'))
            {
                if (seqchar == '\"')
                    seqchar = K_SPACE;  // reset separating char to space

                if (cursor - argptr > 0)
                {
                    m_argv[m_argc] = (char *)mem::alloc( cursor - argptr + 1 );
                    strncpy( m_argv[m_argc], argptr, cursor - argptr );
                    *((m_argv[m_argc])+(cursor-argptr)) = '\0'; // add escape char

                    m_argc++;
                }

                argptr = cursor + 1;    // first char of next sequence
            }
            else if (*cursor == '\"')
            {
                seqchar = '\"';         // set the separating char to '\"'
                argptr = cursor + 1;    // first char inside quote set
            }
        } while (*cursor++);

        return ERROR_NONE;
    }

    char *getline (char *in, char *out, int maxlen)
    {
        char *cursor = in;
        int         length;

        do
        {
            length = cursor - in;
            if ( (*cursor == '\n' || *cursor == '\r' || *cursor == '\0') && length > 0 )
            {
                if ( length > maxlen )
                    length = maxlen;

                if ( length )
                {
                    strncpy( out, in, length );
                    out[length] = 0;
                }

                if ( *cursor )
                    return cursor + 1;
                return NULL;
            }
        } while (*cursor++);

        return NULL;
    }
};

class datatools_c
{
private:
    bool    big_endian;
public:
    datatools_c ()
    {
        byte test[2] = {1,0};

        if ( *(unsigned short *)test == 1)
        {
            big_endian = false;
            bigshort = shortswap;
            littleshort = shortnoswap;
            biglong = longswap;
            littlelong = longnoswap;
            bigfloat = floatswap;
            littlefloat = floatnoswap;
        }
        else
        {
            big_endian = true;
            bigshort = shortnoswap;
            littleshort = shortswap;
            biglong = longnoswap;
            littlelong = longswap;
            bigfloat = floatnoswap;
            littlefloat = floatswap;
        }
    }
    ~datatools_c () {}

    short   (*bigshort) (short l);
    short   (*littleshort) (short l);
    int     (*biglong) (int l);
    int     (*littlelong) (int l);
    float   (*bigfloat) (float f);
    float   (*littlefloat) (float f);

private:
    static short    shortswap (short l)
    {
        byte    b1,b2;

        b1 = l&255;
        b2 = (l>>8)&255;

        return (b1<<8) + b2;
    }
    static short    shortnoswap (short l) { return l; }

    static int  longswap (int l)
    {
        byte    b1,b2,b3,b4;

        b1 = l&255;
        b2 = (l>>8)&255;
        b3 = (l>>16)&255;
        b4 = (l>>24)&255;

        return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
    }
    static int  longnoswap (int l) { return l; }

    static float    floatswap (float f)
    {
        union
        {
            float   f;
            byte    b[4];
        } dat1, dat2;
        
        
        dat1.f = f;
        dat2.b[0] = dat1.b[3];
        dat2.b[1] = dat1.b[2];
        dat2.b[2] = dat1.b[1];
        dat2.b[3] = dat1.b[0];
        return dat2.f;
    }
    static float    floatnoswap (float f) { return f; }
};

static datatools_c  s_data;

namespace data
{
    inline short littleshort (short l)  { return s_data.littleshort(l); }
    inline short bigshort (short l) { return s_data.bigshort(l); }
    inline int littlelong (int l)   { return s_data.littlelong(l); }
    inline int biglong (int l)      { return s_data.biglong(l); }
    inline float littlefloat (float f)  { return s_data.littlefloat(f); }
    inline float bigfloat (float f)     { return s_data.bigfloat(f); }
}

/*
===========================================================

Name    :   cTextBuffer

===========================================================
*/

class textbuf_c
{
public:
    void    Init (char *pBuffer, int nMaxSize) { m_pBuffer = pBuffer ; m_nMaxSize = nMaxSize ; m_nCursorRead = m_nCursorWrite = 0 ; }
    void    Write (char *szIn)
    {
        bool    bLessThan;

        while ( true )
        {
            if ( m_nCursorRead >= m_nMaxSize )
                m_nCursorRead = 1;

            bLessThan = (m_nCursorWrite < m_nCursorRead);

            while ( *szIn && (m_nCursorWrite < m_nMaxSize) )
                m_pBuffer[m_nCursorWrite++] = *szIn++;

            if ( bLessThan && (m_nCursorWrite >= m_nCursorRead) )
                m_nCursorRead = m_nCursorWrite + 1;

            if ( *szIn )
                m_nCursorWrite = 0;
            else
                break;
        }
    }

    int Read (int nCursor, char *szOut)
    {
        int nStart = nCursor;
        int nStop = ( m_nCursorWrite > m_nCursorRead ? m_nCursorRead : 0 );

        while ( nCursor-- > nStop )
        {
            if ( m_pBuffer[nCursor-1] == '\n' || !m_pBuffer[nCursor-1] )
                break;
        }

        if ( nCursor >= 0 )
        {
            strncpy( szOut, m_pBuffer + nCursor, nStart-nCursor );
            szOut[nStart-nCursor] = 0;
            return nCursor;
        }

        nCursor = m_nMaxSize;

        while ( nCursor-- > m_nCursorRead )
        {
            if ( m_pBuffer[nCursor] == '\n' || !m_pBuffer[nCursor] )
                break;
        }

        if ( nCursor > nStart ) // wrapped around
        {
            int     nWrapLength = (m_nMaxSize - nCursor);   // length from max end

            strncpy( szOut, m_pBuffer + nCursor, nWrapLength );
            strncpy( szOut + nWrapLength, m_pBuffer, nStart );
            szOut[nWrapLength + nStart] = 0;
            return nCursor;
        }

        return m_nCursorRead;   // we messed up
    }

    void    CreateLines (char **szLines, int nLines)
    {
        int i = 0;
        int nCursor = m_nCursorWrite;

        for ( i=0 ; i<nLines ; i++ )
        {
            if ( nCursor == m_nCursorRead )
                break;

            szLines[i] = (char *)mem::alloc( LONG_STRING );
            nCursor = Read( nCursor, szLines[i] );
        }
    }

    void    DestroyLines (char **szLines, int nLines)
    {
        int i;

        for ( i=0 ; i<nLines ; i++ )
            if ( szLines[i] )
                mem::free( szLines[i] );
    }
    
private:
    char    *m_pBuffer;
    int m_nMaxSize;
    int m_nCursorRead;
    int m_nCursorWrite;
};
