// r_model.cpp
//

#include "local.h"
#pragma hdrstop

render::model tank_body_model({
    //  center        size          gamma
    {{  0.f,  8.0f}, {20.f,  2.f},  0.5f},  // left tread
    {{  0.f, -8.0f}, {20.f,  2.f},  0.5f},  // right tread
    {{  0.f,  0.0f}, {24.f, 16.f},  0.8f},  // main chassis
    {{- 8.f,  0.0f}, { 4.f, 12.f},  0.6f},  // whatever
    {{-12.f,  5.0f}, { 3.f,  6.f},  0.5f},  // left barrel
    {{-12.f, -5.0f}, { 3.f,  6.f},  0.5f},  // right barrel
    {{ 10.f,  0.0f}, { 2.f, 12.f},  0.6f},
    {{  7.f,  0.0f}, { 2.f, 12.f},  0.6f},
});

render::model tank_turret_model({
    //  center        size          gamma
    {{  0.f,  0.f},  { 8.f,  10.f}, 1.0f},  // turret
    {{  0.f,  0.f},  {10.f,   8.f}, 1.0f},  // turret
    {{ 13.f,  0.f},  {16.f,   2.f}, 1.0f},  // barrel
});

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
model::model(model::rect const* rects, std::size_t num_rects)
    : _list(rects, rects + num_rects)
    , _mins(0,0)
    , _maxs(0,0)
{
    // calculate absolute mins/maxs
    for (auto const& r : _list) {
        vec2 mins = r.center - r.size * 0.5f;
        vec2 maxs = r.center + r.size * 0.5f;

        for (int ii = 0; ii < 2; ++ii) {
            if (_mins[ii] > mins[ii]) {
                _mins[ii] = mins[ii];
            }
            if (_maxs[ii] < maxs[ii]) {
                _maxs[ii] = maxs[ii];
            }
        }
    }
}

//------------------------------------------------------------------------------
void model::draw(vec2 position, float rotation, color4 color) const
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(position.x, position.y, 0);
    glRotatef(rad2deg(rotation), 0, 0, 1);  // YAW

    glBegin(GL_QUADS);

    for (auto const& rect : _list) {
        vec2 rmin = rect.center - rect.size * 0.5f;
        vec2 rmax = rect.center + rect.size * 0.5f;
        color4 rcolor = color * rect.gamma;

        glColor4f(rcolor.r, rcolor.g, rcolor.b, 1.0f);
        glVertex2f(rmin.x, rmin.y);
        glVertex2f(rmax.x, rmin.y);
        glVertex2f(rmax.x, rmax.y);
        glVertex2f(rmin.x, rmax.y);
    }

    glEnd();
    glPopMatrix();
}

} // namespace render
