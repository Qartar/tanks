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
    ~textutils_c () { for( unsigned int i=0 ; i<m_argc ; i++ ) ::free( m_argv[i] ); }

    unsigned int    argc() { return m_argc; }
    char const  *argv(unsigned int argc) { if (argc >= m_argc) return "\0"; return m_argv[argc];    }

    int parse (char const *string)
    {
        char const *cursor;         // current cursor position
        char const *argptr;         // begining of most recent arg
        char    seqchar;            // char that separates an arg

        unsigned int    i;

        // reset the args

        for( i=0 ; i<m_argc ; i++ )
            ::free( m_argv[i] );
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
                    m_argv[m_argc] = (char *)malloc( cursor - argptr + 1 );
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

    char const *getline (char const *in, char *out, int maxlen)
    {
        char const *cursor = in;
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
