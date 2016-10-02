/*
===========================================================

Name    :   oed_files.h

Purpose :   file i/o class

Modified:   11/03/2006

===========================================================
*/

#ifndef __OED_FILES__
#define __OED_FILES__

#include "oed_shared.h"
#include "oed_tools.h"  // [oed_mem.h]

#include <stdio.h>
#include <direct.h>     // _mkdir

#define MAX_READ    0x10000

class filectrl_c
{
private:
    class filerror_c : public errorobj_c
    {
    private:
        filerror_c () {}
    public:
        filerror_c (char *message, bool fatal) : errorobj_c(message,fatal) { strcpy( m_type, "file i/o" ); }
        filerror_c (char *message, int code, bool fatal) : errorobj_c(message,code,fatal) { strcpy( m_type, "file i/o" ); }
    };

public:
    filectrl_c () { m_make_paths(); }
    ~filectrl_c () { m_del_paths(); }

    //
    //  load    -   loads a file into a buffer
    //

    int     load (void **data, char *filename, int *length = NULL)
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

    int open (FILE **file, char *filename, char *attribs)
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

    unsigned int    length (char *filename)
    {
        FILE    *f;
        return m_open( filename, "rb", &f );
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

    int mkdir (char *dir)
    {
        return _mkdir( dir );
    }

    int _remove (char *file)
    {
        return remove( file );
    }
private:
    unsigned int    m_open (char *filename, char *attribs, FILE **file)
    {
        if ( m_num_paths )
        {
            if ( (*file = m_open_paths( filename, attribs )) )
                return length( *file );
        }
        else if ( (*file = fopen( filename, attribs )) )
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

    void m_make_paths ()
    {
        void    *path_file;
        char    *path_cursor;
        char    path_line[MAX_STRING];

        textutils_c text;

        m_num_paths = 0;

        try { load( &path_file, "DATA/DEF/PATHS.DEF" ); }
        catch ( filerror_c ) { return; }

        if ( !path_file )
            return;

        path_cursor = (char *)path_file;
        while ( path_cursor && *path_cursor && m_num_paths < m_max_paths )
        {
            path_cursor = text.getline( path_cursor, path_line, MAX_STRING );

            text.parse( path_line );

            if ( text.argc( ) < 2 )
                continue;

            m_file_paths[m_num_paths].src = (char *)malloc(strlen(text.argv(0))+1);
            m_file_paths[m_num_paths].dst = (char *)malloc(strlen(text.argv(1))+1);

            strncpy( m_file_paths[m_num_paths].src, _strlwr(text.argv(0)), strlen(text.argv(0)) );
            strncpy( m_file_paths[m_num_paths].dst, _strlwr(text.argv(1)), strlen(text.argv(1)) );

            m_file_paths[m_num_paths].src[strlen(text.argv(0))] = 0;
            m_file_paths[m_num_paths].dst[strlen(text.argv(1))] = 0;

            m_num_paths++;
        }

        unload( path_file );
    }

    FILE *m_open_paths (char *file, char *attribs)
    {
        char    file_lwr[LONG_STRING];
        char    path_file[LONG_STRING];
        char    *src_begin;
        FILE    *out;

        int     src_len, src_offs;
        int     dst_len;
        int     file_len;

        strcpy( file_lwr, file );
        _strlwr( file_lwr );

        for ( int i=0 ; i<m_num_paths ; i++ )
        {
            if ( (src_begin = strstr( file_lwr, m_file_paths[i].src )) )
            {
                src_len = strlen(m_file_paths[i].src);
                dst_len = strlen(m_file_paths[i].dst);
                file_len = strlen(file_lwr);
                src_offs = src_begin - file_lwr;

                memset( path_file, 0, LONG_STRING );

                strncpy( path_file, file_lwr, src_offs );
                strncpy( path_file + src_offs, m_file_paths[i].dst, dst_len );
                strncpy( path_file + src_offs + dst_len, src_begin + src_len, file_len - src_offs - src_len );

                if ( (out = fopen( path_file, attribs )) )
                    return out;
            }
        }

        return NULL;
    }

    void m_del_paths ()
    {
        while ( m_num_paths-- )
        {
            free (m_file_paths[m_num_paths].src);
            free (m_file_paths[m_num_paths].dst);
        }
    }

    static const int    m_max_paths = 32;

    int m_num_paths;
    struct file_path_s
    {
        char    *src;
        char    *dst;
    } m_file_paths[m_max_paths];

};

extern filectrl_c   *s_filectrl_c;

namespace file
{
    inline int      load (void **data, char *filename, int *length = NULL ) { return s_filectrl_c->load(data,filename,length); }
    inline int      open (FILE **file, char *filename, char *attribs)   { return s_filectrl_c->open(file,filename,attribs); }
    inline unsigned int length (FILE *file)                             { return s_filectrl_c->length(file); }
    inline unsigned int length (char *filename)                         { return s_filectrl_c->length(filename); }
    inline int      close (FILE *file)                                  { return s_filectrl_c->close(file); }
    inline int      unload (void *data)                                 { return s_filectrl_c->unload(data); }
    inline int      remove (char *file)                                 { return s_filectrl_c->_remove(file); }
    inline int      mkdir (char *dir)                                   { return s_filectrl_c->mkdir(dir); }
}

#endif // __OED_FILES__
