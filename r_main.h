/*
===============================================================================

Name    :   r_main.h

Purpose :   Rendering controller

Date    :   10/19/2004

===============================================================================
*/

#pragma once

#ifndef _WINDOWS_
typedef struct HFONT__* HFONT;
typedef struct HBITMAP__* HBITMAP;
#endif // _WINDOWS_

namespace render {

/*
===========================================================

Name    :   cFont (r_font.cpp)

Purpose :   OpenGL Font Encapsulation

===========================================================
*/

class font
{
public:
    font(char const* name, int size);
    ~font();

    bool compare(char const* name, int size) const;
    void draw(char const* string, vec2 position, vec4 color) const;
    vec2 size(char const* string) const;

private:
    constexpr static int kNumChars = 256;

    std::string _name;
    int _size;

    HFONT _handle;
    unsigned int _list_base;

    byte _char_width[kNumChars];

    static HFONT _system_font;
    static HFONT _active_font;
};

//------------------------------------------------------------------------------
class image
{
public:
    image(char const* name);
    ~image();

    std::string const& name() const { return _name; }
    unsigned int texnum() const { return _texnum; }
    int width() const { return _width; }
    int height() const { return _height; }

protected:
    std::string _name;
    unsigned int _texnum;
    int _width;
    int _height;

protected:
    HBITMAP load_resource(char const* name) const;
    HBITMAP load_file(char const* name) const;

    bool upload(HBITMAP bitmap);
};

} // namespace render

/*
===========================================================

Name    :   cRender (r_main.cpp)

Purpose :   Rendering controller object

===========================================================
*/

class cRender
{
public:
    cRender () {}
    ~cRender () {}

    int     Init ();
    int     Shutdown ();

    void    BeginFrame ();
    void    EndFrame ();

    void    Resize () { m_setDefaultState( ) ; }

    // Font Interface (r_font.cpp)

    render::font const* load_font(char const* szName, int nSize);

    //  Image Interface (r_image.cpp)
    render::image const* load_image(const char *name);
    void draw_image(render::image const* img, vec2 org, vec2 sz, vec4 color);

    // Drawing Functions (r_draw.cpp)

    void draw_string(char const* string, vec2 position, vec4 color);
    vec2 string_size(char const* string) const;

    void    DrawLine (vec2 vOrg, vec2 vEnd, vec4 vColorO, vec4 vColorE);
    void    DrawBox (vec2 vSize, vec2 vPos, float flAngle, vec4 vColor);
    void    DrawParticles (float time, render::particle const* particles, std::size_t num_particles);

    void    SetViewOrigin (vec2 vPos) { m_viewOrigin = vPos ; m_setDefaultState( ) ; }

private:

    // More font stuff (r_font.cpp)

    std::vector<std::unique_ptr<render::font>> _fonts;

    std::vector<std::unique_ptr<render::image>> _images;

    // Internal stuff

    void    m_setDefaultState ();

    vec2    m_viewOrigin;

    bool    m_bATI;

    float   costbl[360];
    float   sintbl[360];
};
