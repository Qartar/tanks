// r_draw.cpp
//

#include "local.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
void system::draw_string(char const* string, vec2 position, vec4 color)
{
    _fonts[0]->draw(string, position, color);
}

//------------------------------------------------------------------------------
vec2 system::string_size(char const* string) const
{
    return _fonts[0]->size(string);
}

//------------------------------------------------------------------------------
void system::draw_box(vec2 size, vec2 position, vec4 color)
{
    float   xl, xh, yl, yh;

    glColor4fv(color.v);

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
    render::particle const* end = particles + num_particles;
    for (render::particle const*p = particles; p < end; ++p) {
        float ptime = time - p->time;

        float vtime = p->drag ? tanhf(p->drag * ptime) / p->drag : ptime;

        float radius = p->size + p->size_velocity * ptime;
        vec4 color = p->color + p->color_velocity * ptime;
        vec2 position = p->position
                      + p->velocity * vtime
                      + p->acceleration * 0.5f * vtime * vtime;

        vec4 color_in = p->flags & render::particle::invert ? color * vec4(1,1,1,0.25f) : color;
        vec4 color_out = p->flags & render::particle::invert ? color : color * vec4(1,1,1,0.25f);

        glBegin(GL_TRIANGLE_FAN);

        // Number of circle segments, approximation for pi / acos(1 - 1/2x)
        int n = 1 + M_PI * sqrtf(radius - 0.25f);
        int k = std::max<int>(1, 360 / n);

        glColor4fv(color_in.v);
        glVertex2fv(position.v);

        if (!(p->flags & render::particle::tail)) {
            // draw circle outline
            glColor4fv(color_out.v);
            for (int ii = 0; ii < 360 ; ii += k) {
                vec2 vertex = position + vec2(_costbl[ii], _sintbl[ii]) * radius;
                glVertex2fv(vertex.v);
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
            int k0 = std::min<int>(1, 360 / n0);

            glColor4fv(color_in.v);
            for (int ii = 0; ii < 360; ii += k0) {
                if (ii < 180) {
                    // draw forward-facing half-circle
                    vec2 vertex = position + (tangent * _costbl[ii] + normal * _sintbl[ii]) * radius;
                    glVertex2fv(vertex.v);
                } else {
                    // draw backward-facing elliptical tail
                    float alpha = -_sintbl[ii];
                    vec4 vcolor = color_out * alpha + color_in * (1.0f - alpha);
                    vec2 vertex = position + tangent * _costbl[ii] * radius + normal * _sintbl[ii] * distance;

                    glColor4fv(vcolor.v);
                    glVertex2fv(vertex.v);
                }
            }

            glColor4fv(color_in.v);
            glVertex2f(position.x + tangent.x * radius, position.y + tangent.y * radius);
        }

        glEnd();
    }
}

//------------------------------------------------------------------------------
void system::draw_line(vec2 start, vec2 end, vec4 start_color, vec4 end_color)
{
    glBegin(GL_LINES);
        glColor4fv(start_color.v);
        glVertex2fv(start.v);
        glColor4fv(start_color.v);
        glVertex2fv(end.v);
    glEnd();
}

} // namespace render
