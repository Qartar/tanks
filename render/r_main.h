// r_main.h
//

#pragma once

#include "cm_config.h"
#include "cm_time.h"

#ifndef _WINDOWS_
typedef struct HFONT__* HFONT;
typedef struct HBITMAP__* HBITMAP;
#endif // _WINDOWS_

typedef unsigned int GLuint;

////////////////////////////////////////////////////////////////////////////////
namespace render {

class window;

//------------------------------------------------------------------------------
class font
{
public:
    font(char const* name, int size);
    ~font();

    bool compare(char const* name, int size) const;
    void draw(char const* string, vec2 position, color4 color, vec2 scale) const;
    vec2 size(char const* string, vec2 scale) const;

private:
    constexpr static int kNumChars = 256;

    std::string _name;
    int _size;

    HFONT _handle;
    unsigned int _list_base;

    short _char_width[kNumChars];

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

//------------------------------------------------------------------------------
struct view
{
    vec2 origin; //!< center
    vec2 size;
    rect viewport;
};

//------------------------------------------------------------------------------
class system
{
public:
    system(render::window* window);

    result init();
    result shutdown();

    void begin_frame();
    void end_frame();

    void resize(vec2i size);
    render::window const* window() const { return _window; }

    render::view const& view() const { return _view; }

    // Font Interface (r_font.cpp)

    render::font const* load_font(char const* szName, int nSize);

    //  Image Interface (r_image.cpp)
    render::image const* load_image(char const* name);
    void draw_image(render::image const* img, vec2 org, vec2 sz, color4 color = color4(1,1,1,1));

    // Drawing Functions (r_draw.cpp)

    void draw_string(char const* string, vec2 position, color4 color);
    vec2 string_size(char const* string) const;

    void draw_monospace(char const* string, vec2 position, color4 color);
    vec2 monospace_size(char const* string) const;

    void draw_line(vec2 start, vec2 end, color4 start_color, color4 end_color);
    void draw_box(vec2 size, vec2 position, color4 color);
    void draw_particles(time_value time, render::particle const* particles, std::size_t num_particles);
    void draw_model(render::model const* model, mat3 transform, color4 color);

    void set_view(render::view const& view);

private:

    // More font stuff (r_font.cpp)

    std::unique_ptr<render::font> _default_font;
    std::unique_ptr<render::font> _monospace_font;

    std::vector<std::unique_ptr<render::font>> _fonts;

    std::vector<std::unique_ptr<render::image>> _images;

    // Internal stuff

    config::integer _framebuffer_width;
    config::integer _framebuffer_height;
    config::scalar _framebuffer_scale;
    config::integer _framebuffer_samples;

    GLuint _fbo;
    GLuint _rbo[2];
    vec2i _framebuffer_size;

    render::window* _window;

    void create_default_font();
    void set_default_state();
    void create_framebuffer(vec2i size, int samples);
    void destroy_framebuffer();

    render::view _view;

    float _costbl[360];
    float _sintbl[360];
};

} // namespace render
