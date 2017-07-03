/*
===============================================================================

Name    :   g_object.cpp

Purpose :   implementation of object class

===============================================================================
*/

#include "local.h"
#pragma hdrstop

physics::material cObject::_default_material(0.5f, 0.5f);
physics::circle_shape cObject::_default_shape(0.5f);

cObject::cObject ()
    : pModel(NULL)
    , eType(object_object)
    , _rigid_body(&_default_shape, &_default_material, _default_mass)
{}

void cObject::Touch (cObject *pOther, float impulse)
{
    return;
}

void cObject::Draw ()
{
    if (pModel) {
        g_Application->get_glWnd()->get_Render()->DrawModel(
            pModel,
            _rigid_body.get_position(),
            _rigid_body.get_rotation(),
            vColor);
    }
}

void cObject::Think ()
{
    // regular objects just sit there and act stupid

    return;
}

vec2 cObject::get_position(float lerp) const
{
    return oldPos + (get_position() - oldPos) * lerp;
}

float cObject::get_rotation(float lerp) const
{
    return oldAngle + (get_rotation() - oldAngle) * lerp;
}
