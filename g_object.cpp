/*
===============================================================================

Name    :   g_object.cpp

Purpose :   implementation of object class

===============================================================================
*/

#include "local.h"
#pragma hdrstop

namespace game {

physics::material object::_default_material(0.5f, 0.5f);
physics::circle_shape object::_default_shape(0.5f);

//------------------------------------------------------------------------------
object::object(object_type type, object* owner)
    : _model(NULL)
    , _type(type)
    , _owner(owner)
    , _rigid_body(&_default_shape, &_default_material, _default_mass)
{}

//------------------------------------------------------------------------------
void object::touch(object* /*other*/, float /*impulse = 0*/)
{
}

//------------------------------------------------------------------------------
void object::draw() const
{
    if (_model) {
        g_Application->get_glWnd()->get_Render()->DrawModel(
            _model,
            _rigid_body.get_position(),
            _rigid_body.get_rotation(),
            _color);
    }
}

//------------------------------------------------------------------------------
void object::think()
{
}

//------------------------------------------------------------------------------
vec2 object::get_position(float lerp) const
{
    return _old_position + (get_position() - _old_position) * lerp;
}

//------------------------------------------------------------------------------
float object::get_rotation(float lerp) const
{
    return _old_rotation + (get_rotation() - _old_rotation) * lerp;
}

} // namespace game
