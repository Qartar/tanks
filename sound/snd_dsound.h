/*=========================================================
Name    :   snd_dsound.h
Date    :   04/04/2006
=========================================================*/

#pragma once

#include <windows.h>
#include <dsound.h>

#include "cm_sound.h"
#include "cm_variable.h"

/*=========================================================
=========================================================*/

typedef HRESULT iDirectSoundCreate (GUID *, LPDIRECTSOUND8 *, IUnknown *);

#define DEFAULT_BUFFER_SIZE 0x10000

class cDirectSoundDevice : public cAudioDevice
{
public:
    cDirectSoundDevice () { cDirectSoundDevice(NULL); }
    cDirectSoundDevice (HWND hWnd);

    virtual void        Destroy ( );

    virtual device_state_t  getState ( ) { return m_State; }
    virtual buffer_info_t   getBufferInfo ( );
    virtual void            writeToBuffer (byte *pAudioData, int nBytes);

private:
    LPDIRECTSOUND8      pDirectSound;
    LPDIRECTSOUNDBUFFER pSoundBuffer;
    LPDIRECTSOUNDBUFFER pPrimaryBuffer;
    WAVEFORMATEX        m_BufferFormat;
    DSCAPS              m_DeviceCaps;
    DSBCAPS             m_BufferCaps;
    int                 m_nOffset;

    int                 CreateBuffers ();
    void                DestroyBuffers ();

    device_state_t      m_State;
    buffer_info_t       m_Info;

    cvar_t          *snd_primary;
    cvar_t          *snd_dsfocus;
    cvar_t          *snd_frequency;

    HINSTANCE       hDirectSound;
    HWND            m_hWnd;
};
