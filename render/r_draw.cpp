// r_draw.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
void system::draw_string(char const* string, vec2 position, color4 color)
{
    _default_font->draw(string, position, color);
}

//------------------------------------------------------------------------------
vec2 system::string_size(char const* string) const
{
    vec2 scale(_view.size.x / _window->framebuffer_size().x,
               _view.size.y / _window->framebuffer_size().y);
    return _default_font->size(string) * scale;
}

//------------------------------------------------------------------------------
void system::draw_box(vec2 size, vec2 position, color4 color)
{
    float   xl, xh, yl, yh;

    glColor4fv(color);

    xl = position.x - size.x / 2;
    xh = position.x + size.x / 2;
    yl = position.y - size.y / 2;
    yh = position.y + size.y / 2;

    glBegin(GL_QUADS);
        glVertex2f(xl, yl);
        glVertex2f(xh, yl);
        glVertex2f(xh, yh);
        glVertex2f(xl, yh);
    glEnd();
}

//------------------------------------------------------------------------------
void system::draw_particles(float time, render::particle const* particles, std::size_t num_particles)
{
    // Scaling factor for particle tessellation
    const float view_scale = sqrtf(_window->framebuffer_size().length_sqr() / _view.size.length_sqr());

    render::particle const* end = particles + num_particles;
    for (render::particle const*p = particles; p < end; ++p) {
        float ptime = time - p->time;

        if (ptime < 0) {
            continue;
        }

        float vtime = p->drag ? tanhf(p->drag * ptime) / p->drag : ptime;

        float radius = p->size + p->size_velocity * ptime;
        color4 color = p->color + p->color_velocity * ptime;
        vec2 position = p->position
                      + p->velocity * vtime
                      + p->acceleration * 0.5f * vtime * vtime;

        color4 color_in = p->flags & render::particle::invert ? color * color4(1,1,1,0.25f) : color;
        color4 color_out = p->flags & render::particle::invert ? color : color * color4(1,1,1,0.25f);

        glBegin(GL_TRIANGLE_FAN);

        // Number of circle segments, approximation for pi / acos(1 - 1/2x)
        int n = 1 + M_PI * sqrtf(radius * view_scale - 0.25f);
        int k = std::max<int>(1, 360 / n);

        glColor4fv(color_in);
        glVertex2fv(position);

        if (!(p->flags & render::particle::tail)) {
            // draw circle outline
            glColor4fv(color_out);
            for (int ii = 0; ii < 360 ; ii += k) {
                vec2 vertex = position + vec2(_costbl[ii], _sintbl[ii]) * radius;
                glVertex2fv(vertex);
            }
            glVertex2f(position.x + radius, position.y);
        } else {
            float tail_time = std::max<float>(0.0f, time - p->time - FRAMETIME);
            float tail_vtime = p->drag ? tanhf(p->drag * tail_time) / p->drag : tail_time;

            vec2 tail_position = p->position
                               + p->velocity * tail_vtime
                               + p->acceleration * 0.5f * tail_vtime * tail_vtime;

            // calculate forward and tangent vectors
            vec2 normal = position - tail_position;
            float distance = normal.length();
            normal /= distance;
            distance = std::max<float>(distance, radius);
            vec2 tangent = vec2(-normal.y, normal.x);

            // particle needs at least 4 verts to look reasonable
            int n0 = std::max<int>(4, n);
            int k0 = std::max<int>(1, 360 / n0);

            glColor4fv(color_in);
            for (int ii = 0; ii < 360; ii += k0) {
                if (ii < 180) {
                    // draw forward-facing half-circle
                    vec2 vertex = position + (tangent * _costbl[ii] + normal * _sintbl[ii]) * radius;
                    glVertex2fv(vertex);
                } else {
                    // draw backward-facing elliptical tail
                    float alpha = -_sintbl[ii];
                    color4 vcolor = color_out * alpha + color_in * (1.0f - alpha);
                    vec2 vertex = position + tangent * _costbl[ii] * radius + normal * _sintbl[ii] * distance;

                    glColor4fv(vcolor);
                    glVertex2fv(vertex);
                }
            }

            glColor4fv(color_in);
            glVertex2f(position.x + tangent.x * radius, position.y + tangent.y * radius);
        }

        glEnd();
    }
}

//------------------------------------------------------------------------------
void system::draw_line(vec2 start, vec2 end, color4 start_color, color4 end_color)
{
    glBegin(GL_LINES);
        glColor4fv(start_color);
        glVertex2fv(start);
        glColor4fv(end_color);
        glVertex2fv(end);
    glEnd();
}

} // namespace render
