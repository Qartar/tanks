/*
===========================================================

Name    :   oed_files.h

Purpose :   file i/o class

Modified:   11/03/2006

===========================================================
*/

#pragma once

#include "oed_shared.h"
#include "oed_tools.h"  // [oed_mem.h]

#include <stdio.h>
#include <direct.h>     // _mkdir

#define MAX_READ    0x10000

class filectrl_c
{
public:
    //
    //  load    -   loads a file into a buffer
    //

    int     load (void **data, char const *filename, int *length = NULL)
    {
        FILE            *file;
        byte            *buffer;
        unsigned int    len;

        if (!data) {
            return 0;
        }

        len = m_open( filename, "rb", &file );

        if (!file) {
            *data = NULL;
            return 0;
        }

        buffer = (byte *)mem::alloc( len+1 );
        m_read( buffer, len, file );

        buffer[len] = 0;

        *data = buffer;

        close( file );

        if ( length )
            *length = len;

        return ERROR_NONE;
    }

    //
    //  open    -   opens a file with given attributes
    //

    int open (FILE **file, char const *filename, char const *attribs)
    {
        FILE    *temp;

        m_open( filename, attribs, &temp );

        *file = temp;

        return (file == NULL);
    }

    unsigned int    length (FILE *file)
    {
        int     pos, end;

        pos = ftell( file );
        fseek( file, 0, SEEK_END );
        end = ftell( file );
        fseek( file, pos, SEEK_SET );

        return end;
    }

    unsigned int    length (char const *filename)
    {
        FILE    *f;
        unsigned int len = m_open( filename, "rb", &f );
        if ( f )
            close( f );
        return len;
    }

    int close (FILE *file)
    {
        return fclose( file );
    }

    int unload (void *data)
    {
        mem::free( data );

        return ERROR_NONE;
    }

    int mkdir (char const *dir)
    {
        return _mkdir( dir );
    }

    int _remove (char const *file)
    {
        return remove( file );
    }
private:
    unsigned int    m_open (char const *filename, char const *attribs, FILE **file)
    {
        if ( (*file = fopen( filename, attribs )) )
            return length( *file );

        return 0;
    }

    int m_read (void *data, unsigned int length, FILE *file)
    {
        int     block, remaining;
        int     read;
        byte    *buffer;

        buffer = (byte *)data;

        remaining = length;
        while (remaining)
        {
            block = remaining;
            if (block > MAX_READ)
                block = MAX_READ;
            read = fread( buffer, 1, block, file );

            if (read < 1)
                return ERROR_NONE;

            remaining -= read;
            buffer += read;
        }

        return ERROR_NONE;
    }
};

extern filectrl_c   *s_filectrl_c;

namespace file
{
    inline int      load (void **data, char const *filename, int *length = NULL ) { return s_filectrl_c->load(data,filename,length); }
    inline int      open (FILE **file, char const *filename, char const *attribs) { return s_filectrl_c->open(file,filename,attribs); }
    inline unsigned int length (FILE *file)                             { return s_filectrl_c->length(file); }
    inline unsigned int length (char const *filename)                   { return s_filectrl_c->length(filename); }
    inline int      close (FILE *file)                                  { return s_filectrl_c->close(file); }
    inline int      unload (void *data)                                 { return s_filectrl_c->unload(data); }
    inline int      remove (char const *file)                           { return s_filectrl_c->_remove(file); }
    inline int      mkdir (char const *dir)                             { return s_filectrl_c->mkdir(dir); }
}
