/*
===============================================================================

Name	:	g_object.cpp

Purpose	:	implementation of object class

===============================================================================
*/

#include "local.h"

cObject::cObject ()
{
	flAngle = 0;
	flAVel = 0;

	vVel = vec2(0,0);

	eType = object_object;
}

void cObject::Touch (cObject *pOther)
{
	return;
}

void cObject::Draw ()
{
	g_Application->get_glWnd()->get_Render()->DrawModel( pModel, vPos, flAngle, vColor );
}

void cObject::Think ()
{
	// regular objects just sit there and act stupid

	return;
}