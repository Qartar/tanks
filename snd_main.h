/*
===============================================================================

Name	:	snd_main.h

Purpose	:	sound header file

Date	:	08/17/2005

===============================================================================
*/

#include "shared.h"

#include <fmod.hpp>
#include <fmod_errors.h>

class cSoundObject
{
public:
	char			szName[LONG_STRING];
	cSoundObject	*pNext;
	FMOD::Sound		*pSound;
};

class cChannelObject
{
public:
	int				nChannelId;
	cChannelObject	*pNext;
	FMOD::Channel	*pChannel;
};

class cSound
{
public:
	cSound () { m_pSystem = NULL; }

	int		Init ();
	int		Shutdown ();

	void	Update (vec2 vOrigin);

	void	Activate (bool bActive);

	void	Register (char *szName);
	void	StartSound (char *szName);

	int		StartLoop (char *szName);
	void	StopLoop (int nChannel);
	void	LoopFrequency (int nChannel, float flFreq);

	void	StopAll ();
    
private:
	cSoundObject	m_SoundChain;
	cChannelObject	m_ChannelChain;

	FMOD::System	*m_pSystem;

	int				m_nChannel;

	cChannelObject	*m_AllocChannel (void);
	void			m_DeleteChannel (cChannelObject *pObject);

	cChannelObject	*m_FindChannel (int nChannel);
};

extern cSound	g_Sound;