/*
===============================================================================

Name    :   g_object.cpp

Purpose :   implementation of object class

===============================================================================
*/

#include "local.h"
#pragma hdrstop

cObject::cObject ()
{
    eType = object_object;
}

void cObject::Touch (cObject *pOther)
{
    return;
}

void cObject::Draw ()
{
    g_Application->get_glWnd()->get_Render()->DrawModel(
        pModel,
        _rigid_body->get_position(),
        _rigid_body->get_rotation(),
        vColor);
}

void cObject::Think ()
{
    // regular objects just sit there and act stupid

    return;
}

vec2 cObject::GetPos( float lerp ) {
    return oldPos + ( _rigid_body->get_position() - oldPos ) * lerp;
}

float cObject::GetAngle( float lerp ) {
    return oldAngle + ( _rigid_body->get_rotation() - oldAngle ) * lerp;
}
