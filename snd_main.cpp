/*
===============================================================================

Name	:	snd_main.cpp

Purpose	:	3rd party sound lib interface

Date	:	08/18/2005

===============================================================================
*/

#include "snd_main.h"

#include <windows.h>
#include <string.h>
#include <stdlib.h>

cSound	g_Sound;

int ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		MessageBox( NULL, va("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result)), "FMOD Error", MB_OK );
		return ERROR_FAIL;
	}
	return ERROR_NONE;
}

int cSound::Init ()
{
	unsigned int	nVersion;

	m_pSystem = NULL;

	if ( ERRCHECK( FMOD::System_Create( &m_pSystem ) ) != ERROR_NONE )
		return ERROR_NONE;

	if ( ERRCHECK( m_pSystem->getVersion( &nVersion ) ) != ERROR_NONE )
		return ERROR_FAIL;

	if ( nVersion != FMOD_VERSION )
	{
		MessageBox( NULL, "Bad Library Version, not using sound", "FMOD Error", MB_OK );

		m_pSystem->close( );
		m_pSystem->release( );

		return ERROR_FAIL;
	}

	ERRCHECK( m_pSystem->init( 32, FMOD_INIT_NORMAL, NULL ) );

	m_SoundChain.pNext = NULL;
	m_ChannelChain.pNext = NULL;
	m_nChannel = 0;

	// these should be moved to g_world or g_main
	Register( "SOUND/TANK_EXPLODE.wav" );
	Register( "SOUND/TANK_FIRE.wav" );
	Register( "SOUND/TANK_MOVE.wav" );
	Register( "SOUND/TANK_IDLE.wav" );

	Register( "SOUND/TURRET_MOVE.wav" );
	Register( "SOUND/BULLET_EXPLODE.wav" );

	return ERROR_NONE;
}

int cSound::Shutdown ()
{
	cSoundObject	*pObject, *pNext;

	if ( !m_pSystem )
		return ERROR_NONE;

	pObject = m_SoundChain.pNext;

	while ( pObject )
	{
		pNext = pObject->pNext;

		pObject->pSound->release( );

		delete pObject;

		pObject = pNext;
	}

	StopAll( );

	m_pSystem->close( );
	m_pSystem->release( );
	m_pSystem = NULL;

	return ERROR_NONE;
}

void cSound::Update (vec2 vOrigin)
{
	if ( !m_pSystem )
		return;

	m_pSystem->update( );
}

void cSound::Activate (bool bActive)
{
	return;
}

void cSound::Register (char *szName)
{
	cSoundObject	*pObject;

	if ( !m_pSystem )
		return;

	pObject = m_SoundChain.pNext;
	while ( pObject )
	{
		if ( stricmp( szName, pObject->szName ) == 0 )
			return;

		pObject = pObject->pNext;
	}

	pObject = new cSoundObject;

	pObject->pNext = m_SoundChain.pNext;
	m_SoundChain.pNext = pObject;

	strcpy( pObject->szName, szName );
	ERRCHECK( m_pSystem->createSound( szName, FMOD_HARDWARE, NULL, &pObject->pSound ) );

	return;
}

void cSound::StartSound (char *szName)
{
	cSoundObject *pObject;

	if ( !m_pSystem )
		return;

	pObject = m_SoundChain.pNext;
	while ( pObject )
	{
		if ( stricmp( szName, pObject->szName ) == 0 )
		{
			ERRCHECK( m_pSystem->playSound( FMOD_CHANNEL_FREE, pObject->pSound, 0, NULL ) );
			return;
		}

		pObject = pObject->pNext;
	}

	Register( szName );

	return;
}

int cSound::StartLoop (char *szName)
{
	cSoundObject *pObject;
	cChannelObject	*pChannel;

	if ( !m_pSystem )
		return -1;

	pChannel = m_AllocChannel( );

	pObject = m_SoundChain.pNext;
	while ( pObject )
	{
		if ( stricmp( szName, pObject->szName ) == 0 )
		{
			ERRCHECK( m_pSystem->playSound( FMOD_CHANNEL_FREE, pObject->pSound, true, &pChannel->pChannel ) );

			pChannel->pChannel->setMode( FMOD_LOOP_NORMAL );
			pChannel->pChannel->setPaused( false );

			return pChannel->nChannelId;
		}

		pObject = pObject->pNext;
	}

	Register( szName );

	return -1;
}

void cSound::StopLoop (int nChannel)
{
	cChannelObject	*pObject;

	pObject = m_FindChannel( nChannel );

	if ( pObject )
		pObject->pChannel->stop( );

	m_DeleteChannel( pObject );

	return;
}

void cSound::LoopFrequency (int nChannel, float flFreq)
{
	cChannelObject	*pObject;

	pObject = m_FindChannel( nChannel );

	if ( pObject )
		pObject->pChannel->setFrequency( flFreq );

	return;
}

void cSound::StopAll ()
{
	cChannelObject	*pObject, *pNext;

	pObject = m_ChannelChain.pNext;
	while ( pObject )
	{
		pNext = pObject->pNext;

		pObject->pChannel->stop( );
		m_DeleteChannel( pObject );

		pObject = pNext;
	}
	
	return;
}


cChannelObject *cSound::m_AllocChannel (void)
{
	cChannelObject	*pObject = new cChannelObject;

	pObject->nChannelId = m_nChannel++;

	pObject->pNext = m_ChannelChain.pNext;
	m_ChannelChain.pNext = pObject;

	return pObject;
}

void cSound::m_DeleteChannel (cChannelObject *pObject)
{
	cChannelObject	*pNext, *pLast = NULL;

	if ( !pObject )
		return;

	if ( m_ChannelChain.pNext == pObject )
	{
		m_ChannelChain.pNext = pObject->pNext;

		delete pObject;
		return;
	}

	pNext = m_ChannelChain.pNext;
	while ( pNext )
	{
		if ( pNext == pObject )
		{
			pLast->pNext = pObject->pNext;

			delete pObject;
			return;
		}

		pLast = pNext;
		pNext = pNext->pNext;
	}

	return;
}

cChannelObject *cSound::m_FindChannel (int nChannel)
{
	cChannelObject	*pObject;

	pObject = m_ChannelChain.pNext;
	while ( pObject )
	{
		if ( pObject->nChannelId == nChannel )
			return pObject;

		pObject = pObject->pNext;
	}

	return NULL;
}