// r_main.cpp
//

#include "precompiled.h"
#pragma hdrstop

namespace {

// additional opengl bindings
typedef void (APIENTRY* PFNGLBINDRENDERBUFFER)(GLenum target, GLuint renderbuffer);
typedef void (APIENTRY* PFNGLDELETERENDERBUFFERS)(GLsizei n, GLuint const* renderbuffers);
typedef void (APIENTRY* PFNGLGENRENDERBUFFERS)(GLsizei n, GLuint* renderbuffers);
typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGE)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLE)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY* PFNGLBINDFRAMEBUFFER)(GLenum target, GLuint framebuffer);
typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERS)(GLsizei n, GLuint const* framebuffers);
typedef void (APIENTRY* PFNGLGENFRAMEBUFFERS)(GLsizei n, GLuint* framebuffers);
typedef void (APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFER)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRY* PFNGLBLITFRAMEBUFFER)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (APIENTRY* PFNGLBLENDCOLOR)(GLfloat red, GLfloat greed, GLfloat blue, GLfloat alpha);

static PFNGLBINDRENDERBUFFER glBindRenderbuffer = NULL;
static PFNGLDELETERENDERBUFFERS glDeleteRenderbuffers = NULL;
static PFNGLGENRENDERBUFFERS glGenRenderbuffers = NULL;
static PFNGLRENDERBUFFERSTORAGE glRenderbufferStorage = NULL;
static PFNGLRENDERBUFFERSTORAGEMULTISAMPLE glRenderbufferStorageMultisample = NULL;
static PFNGLBINDFRAMEBUFFER glBindFramebuffer = NULL;
static PFNGLDELETEFRAMEBUFFERS glDeleteFramebuffers = NULL;
static PFNGLGENFRAMEBUFFERS glGenFramebuffers = NULL;
static PFNGLFRAMEBUFFERRENDERBUFFER glFramebufferRenderbuffer = NULL;
static PFNGLBLITFRAMEBUFFER glBlitFramebuffer = NULL;

#define GL_FRAMEBUFFER                  0x8D40
#define GL_READ_FRAMEBUFFER             0x8CA8
#define GL_DRAW_FRAMEBUFFER             0x8CA9
#define GL_RENDERBUFFER                 0x8D41
#define GL_MAX_SAMPLES                  0x8D57
#define GL_COLOR_ATTACHMENT0            0x8CE0
#define GL_DEPTH_STENCIL_ATTACHMENT     0x821A
#define GL_DEPTH24_STENCIL8             0x88F0

} // anonymous namespace

extern PFNGLBLENDCOLOR glBlendColor; // r_draw.cpp

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
system::system(render::window* window)
    : _framebuffer_width("r_width", 0, config::archive, "framebuffer width, or 0 to use window width")
    , _framebuffer_height("r_height", 0, config::archive, "framebuffer height, or 0 to use window height")
    , _framebuffer_scale("r_scale", 1, config::archive, "framebuffer scale if using window dimensions")
    , _framebuffer_samples("r_samples", -1, config::archive, "framebuffer samples, or -1 to use maximum supported")
    , _window(window)
    , _view{}
    , _fbo(0)
    , _rbo{0, 0}
{}

//------------------------------------------------------------------------------
result system::init()
{
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFER )wglGetProcAddress("glBindRenderbuffer");
    glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERS )wglGetProcAddress("glDeleteRenderbuffers");
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERS )wglGetProcAddress("glGenRenderbuffers");
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGE )wglGetProcAddress("glRenderbufferStorage");
    glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLE )wglGetProcAddress("glRenderbufferStorageMultisample");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFER )wglGetProcAddress("glBindFramebuffer");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERS )wglGetProcAddress("glDeleteFramebuffers");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERS )wglGetProcAddress("glGenFramebuffers");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFER )wglGetProcAddress("glFramebufferRenderbuffer");
    glBlitFramebuffer = (PFNGLBLITFRAMEBUFFER )wglGetProcAddress("glBlitFramebuffer");
    glBlendColor = (PFNGLBLENDCOLOR )wglGetProcAddress("glBlendColor");

    _view.size = vec2(_window->size());
    _view.origin = _view.size * 0.5f;
    _view.viewport = {};

    resize(_window->size());

    for (int ii = 0; ii < 360 ; ++ii) {
        _sintbl[ii] = std::sin(math::deg2rad(static_cast<float>(ii)));
        _costbl[ii] = std::cos(math::deg2rad(static_cast<float>(ii)));
    }

    return result::success;
}

//------------------------------------------------------------------------------
result system::shutdown()
{
    destroy_framebuffer();
    _fonts.clear();
    return result::success;
}

//------------------------------------------------------------------------------
void system::begin_frame()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

//------------------------------------------------------------------------------
void system::end_frame()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glDrawBuffer(GL_BACK);

    glBlitFramebuffer(
        0, 0, _framebuffer_size.x, _framebuffer_size.y,
        0, 0, _window->size().x, _window->size().y,
        GL_COLOR_BUFFER_BIT, GL_LINEAR
    );

    _window->end_frame();

    if (_framebuffer_width.modified()
            || _framebuffer_height.modified()
            || _framebuffer_samples.modified()
            || _framebuffer_scale.modified()) {
        resize(_window->size());
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

//------------------------------------------------------------------------------
void system::set_view(render::view const& view)
{
    _view = view;
    set_default_state();
}

//------------------------------------------------------------------------------
void system::resize(vec2i size)
{
    if (!glGenFramebuffers) {
        return;
    }

    vec2i actual_size;
    actual_size.x = _framebuffer_width ? _framebuffer_width : static_cast<int>(size.x * _framebuffer_scale);
    actual_size.y = _framebuffer_height ? _framebuffer_height : static_cast<int>(size.y * _framebuffer_scale);

    GLint max_samples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    GLint num_samples = actual_size != size ? 0
        : _framebuffer_samples == -1 ? max_samples
        : std::min<int>(max_samples, _framebuffer_samples);

    create_framebuffer(actual_size, num_samples);

    _framebuffer_width.reset();
    _framebuffer_height.reset();
    _framebuffer_samples.reset();
    _framebuffer_scale.reset();

    create_default_font();
    set_default_state();
}

//------------------------------------------------------------------------------
void system::create_default_font()
{
    int size = static_cast<int>((12.f / 480.f) * float(_framebuffer_size.y));

    if (!_default_font || !_default_font->compare("Tahoma", size)) {
        _default_font = std::make_unique<render::font>("Tahoma", size);
    }

    if (!_monospace_font || !_monospace_font->compare("Consolas", size)) {
        _monospace_font = std::make_unique<render::font>("Consolas", size);
    }
}

//------------------------------------------------------------------------------
void system::set_default_state()
{
    glDisable(GL_TEXTURE_2D);

    glClearColor(0, 0, 0, 0.1f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glEnable(GL_POINT_SMOOTH );
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glPointSize(2.0f);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (!_view.viewport.empty()) {
        glViewport(
            _view.viewport.mins().x,
            _view.viewport.mins().y,
            _view.viewport.size().x,
            _view.viewport.size().y
        );
    } else {
        glViewport(0, 0, _framebuffer_size.x, _framebuffer_size.y);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    vec2 view_min = _view.origin - _view.size * 0.5f;
    vec2 view_max = _view.origin + _view.size * 0.5f;

    glOrtho(view_min.x, view_max.x, view_max.y, view_min.y, -99999, 99999);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//------------------------------------------------------------------------------
void system::create_framebuffer(vec2i size, int samples)
{
    if (_fbo) {
        destroy_framebuffer();
    }

    glGenFramebuffers(1, &_fbo);
    glGenRenderbuffers(2, _rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[0]);
    if (samples) {
        glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, GL_RGBA, size.x, size.y);
    } else {
        glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, size.x, size.y);
    }
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo[0]);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[1]);
    if (samples) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, size.x, size.y);
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo[1]);

    _framebuffer_size = size;
}

//------------------------------------------------------------------------------
void system::destroy_framebuffer()
{
    glDeleteRenderbuffers(2, _rbo);
    glDeleteFramebuffers(1, &_fbo);
}

} // namespace render
