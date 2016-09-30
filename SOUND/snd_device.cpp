/*=========================================================
Name	:	snd_device.h
Date	:	04/04/2006
=========================================================*/

#include <windows.h>

#include "snd_main.h"
#include "snd_device.h"
#include "snd_dsound.h"

/*=========================================================
=========================================================*/

cAudioDevice *cAudioDevice::Create (HWND hWnd)
{
	cAudioDevice	*pDevice;
	device_state_t	devState = device_fail;

	pMain->Message( "------ initializing sound ------" );

	//	try directsound
	if ( (pDevice = new cDirectSoundDevice(hWnd)) )
	{
		if ( (devState = pDevice->getState( )) == device_ready )
			return pDevice;

		pDevice->Destroy( );
		delete pDevice;

		if ( devState == device_abort )
			return NULL;
	}
	
//	if ( (pDevice = new X)) )
	

	return NULL;
}

/*=========================================================
=========================================================*/

void cAudioDevice::Destroy (cAudioDevice *pDevice)
{
	pMain->Message( "------ shutting down sound ------" );

	if ( pDevice )
	{
		pDevice->Destroy( );
		delete pDevice;
	}

	return;
}