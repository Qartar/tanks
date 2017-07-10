// net_msg.cpp
//

#include "net_main.h"

#define ANGLE2SHORT(x)  ((int)((x)*(32768.0/360)))
#define SHORT2ANGLE(x)  ((x)*(360.0/32768))

////////////////////////////////////////////////////////////////////////////////
namespace network {

//------------------------------------------------------------------------------
void message::init (byte *buffer, int buffer_size)
{
    memset( this, 0, sizeof(network::message) );
    this->data = buffer;
    this->size = buffer_size;

    this->bytes_remaining = buffer_size;
    this->bytes_written = 0;
}

//------------------------------------------------------------------------------
void message::clear ()
{
    memset( data, 0, bytes_written );
    bytes_written = 0;
    bytes_remaining = size;
    overflowed = false;
}

//------------------------------------------------------------------------------
void *message::alloc (int length)
{
    void    *ret;

    if (bytes_written + length > size)
    {
        overflowed = true;
        return NULL;
    }

    ret = (void *)(data + bytes_written);
    bytes_written += length;
    bytes_remaining -= length;

    return ret;
}

//------------------------------------------------------------------------------
void message::write (void const* buffer, int length)
{
    void    *buf = alloc( length );

    if ( buf )
        memcpy( buf, buffer, length );
}

//------------------------------------------------------------------------------
int message::read (void *buffer, int length)
{
    if ( bytes_read + length > size )
        return -1;

    memcpy( buffer, (void *)(data+bytes_read), length );

    bytes_read += length;

    return 0;
}

//------------------------------------------------------------------------------
void message::write_byte (int b)
{
    byte    *buf = (byte *)alloc( 1 );

    if ( buf )
        buf[0] = b & 0xff;
}

//------------------------------------------------------------------------------
void message::write_short (int s)
{
    byte    *buf = (byte *)alloc( 2 );

    if ( buf )
    {
        buf[0] = s & 0xff;
        buf[1] = s>>8;
    }
}

//------------------------------------------------------------------------------
void message::write_long (int l)
{
    byte    *buf = (byte *)alloc( 4 );

    if ( buf )
    {
        buf[0] = l & 0xff;
        buf[1] = (l>>8) & 0xff;
        buf[2] = (l>>16) & 0xff;
        buf[3] = (l>>24) & 0xff;
    }
}

//------------------------------------------------------------------------------
void message::write_float (float f)
{
    union
    {
        float   f;
        int l;
    } dat;

    dat.f = f;
    dat.l = LITTLE_LONG( dat.l );

    write( &dat.l, 4 );
}

//------------------------------------------------------------------------------
void message::write_char (int b)
{
    char    *buf = (char *)alloc( 1 );

    if ( buf )
        buf[0] = b & 0xff;
}

//------------------------------------------------------------------------------
void message::write_string (char const* sz)
{
    if ( sz )
        write( sz, strlen(sz)+1 );
    else
        write( "", 1 );
}

//------------------------------------------------------------------------------
void message::write_angle (float f)
{
    write_short( ANGLE2SHORT(f) );
}

//------------------------------------------------------------------------------
void message::write_vector (vec2 v)
{
    write_float( v.x );
    write_float( v.y );
}

//------------------------------------------------------------------------------
void message::begin ()
{
    bytes_read = 0;
}

//------------------------------------------------------------------------------
int message::read_byte ()
{
    int b;

    if (bytes_read + 1 > bytes_written)
        b = -1;
    else
        b = (byte )data[bytes_read];

    bytes_read++;

    return b;
}

//------------------------------------------------------------------------------
int message::read_short ()
{
    int s;

    if (bytes_read + 2 > bytes_written)
        s = -1;
    else
        s = (short )data[bytes_read]
        + (data[bytes_read+1]<<8);

    bytes_read += 2;

    return s;
}

//------------------------------------------------------------------------------
int message::read_long ()
{
    int l;

    if (bytes_read + 4 > bytes_written)
        l = -1;
    else
        l = (int )data[bytes_read]
        + (data[bytes_read+1]<<8)
        + (data[bytes_read+2]<<16)
        + (data[bytes_read+3]<<24);

    bytes_read += 4;

    return l;
}

//------------------------------------------------------------------------------
float message::read_float ()
{
    union
    {
        byte    b[4];
        float   f;
        int     l;
    } dat;

    if (bytes_read + 4 > bytes_written)
        dat.l = -1;
    else
    {
        dat.b[0] = data[bytes_read + 0];
        dat.b[1] = data[bytes_read + 1];
        dat.b[2] = data[bytes_read + 2];
        dat.b[3] = data[bytes_read + 3];
    }

    bytes_read += 4;

    dat.l = LITTLE_LONG( dat.l );

    return dat.f;
}

//------------------------------------------------------------------------------
int message::read_char ()
{
    int b;

    if (bytes_read + 1 > bytes_written)
        b = -1;
    else
        b = (char )data[bytes_read];

    bytes_read++;

    return b;
}


//------------------------------------------------------------------------------
char *message::read_string ()
{
    static char string[MAX_STRING];
    int         i,c;

    for (i=0 ; i<MAX_STRING-1 ; i++)
    {
        c = read_byte( );
        if ( c == -1 || c == 0 )
            break;
        string[i] = c;
    }
    string[i] = 0;

    return string;
}


//------------------------------------------------------------------------------
char *message::read_line ()
{
    static char string[MAX_STRING];
    int         i,c;

    for (i=0 ; i<MAX_STRING-1 ; i++)
    {
        c = read_byte( );
        if ( c == -1 || c == 0 || c == '\n')
            break;
        string[i] = c;
    }
    string[i] = 0;

    return string;
}

//------------------------------------------------------------------------------
float message::read_angle ()
{
    int     out;

    out = read_short( );

    return ( SHORT2ANGLE(out) );
}

//------------------------------------------------------------------------------
vec2 message::read_vector ()
{
    float   outx, outy;

    if (bytes_read + 8 > bytes_written)
        return vec2(0,0);

    outx = read_float( );
    outy = read_float( );

    return vec2( outx, outy );
}

} // namespace network
