// r_main.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
int system::init()
{
    set_view_origin(vec2(0,0));

    _fonts.push_back(std::make_unique<render::font>("Tahoma", 12));

    for (int ii = 0; ii < 360 ; ++ii) {
        _sintbl[ii] = sin(deg2rad<float>(ii));
        _costbl[ii] = cos(deg2rad<float>(ii));
    }

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int system::shutdown()
{
    _fonts.clear();

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
void system::begin_frame()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

//------------------------------------------------------------------------------
void system::end_frame()
{
    _window->end_frame();
}

//------------------------------------------------------------------------------
void system::_set_default_state()
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

    glViewport(0, 0, DEFAULT_W, DEFAULT_H);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, DEFAULT_W, DEFAULT_H, 0, -99999, 99999);

    glTranslatef(
        -_view_origin.x,
        -_view_origin.y,
        0 );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

} // namespace render
