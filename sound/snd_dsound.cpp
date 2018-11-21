/*=========================================================
Name    :   snd_dsound.cpp
Date    :   04/04/2006
=========================================================*/

#include "snd_main.h"
#include "snd_device.h"
#include "snd_dsound.h"

#include <dsound.h>

HRESULT (WINAPI *pDirectSoundCreate)(GUID *, LPDIRECTSOUND8 *, IUnknown *);

/*=========================================================
=========================================================*/

cDirectSoundDevice::cDirectSoundDevice (HWND hWnd)
    : snd_primary("snd_primary", false, config::archive, "use primary sound buffer")
    , snd_dsfocus("snd_dsfocus", true, config::archive, "")
    , snd_frequency("snd_frequency", 22050, config::archive, "sound playback speed")
{
    HRESULT         hResult;

    pMain->message( "attempting to use DirectSound..." );

    pDirectSound = NULL;
    pSoundBuffer = NULL;
    hDirectSound = NULL;

    m_hWnd = hWnd;

    m_State = device_fail;

    pMain->message( "...linking dsound.dll: " );

    if ( (hDirectSound = LoadLibrary( "dsound.dll" )) == NULL )
    {
        pMain->message( "failed" );
        return;
    }

    if ( (pDirectSoundCreate = (HRESULT (__stdcall *)(GUID *, LPDIRECTSOUND8 *, IUnknown *))GetProcAddress( hDirectSound, "DirectSoundCreate8" )) == NULL )
    {
        pMain->message( "...GetProcAddress failed" );
        return;
    }

    pMain->message( "...creating DirectSound object: " );
    if ( ( hResult = pDirectSoundCreate( NULL, &pDirectSound, NULL )) != DS_OK )
    {
        if ( hResult == DSERR_ALLOCATED )
        {
            pMain->message( "failed, device in use" );
            m_State = device_abort;
        }
        else
            pMain->message( "failed" );

        return;
    }

    m_DeviceCaps.dwSize = sizeof(DSCAPS);
    if ( pDirectSound->GetCaps( &m_DeviceCaps ) != DS_OK )
    {
        pMain->message( "...GetCaps failed" );
        return;
    }

    if ( m_DeviceCaps.dwFlags & DSCAPS_EMULDRIVER )
    {
        pMain->message( "...sound drivers not present" );
        return;
    }

    if ( failed( CreateBuffers() ) )
        return;

    pMain->message( "...completed successfully" );
    m_State = device_ready;
}

/*=========================================================
=========================================================*/

void cDirectSoundDevice::Destroy ()
{
    pMain->message( "shutting down DirectSound..." );

    if ( pSoundBuffer )
        DestroyBuffers( );

    if ( pDirectSound )
    {
        pMain->message( "...releasing DirectSound object" );

        pDirectSound->Release( );
        pDirectSound = NULL;
    }

    if ( hDirectSound )
    {
        pMain->message( "...releasing dsound.dll" );

        FreeLibrary( hDirectSound );
        hDirectSound = NULL;
    }
}

/*=========================================================
=========================================================*/

result cDirectSoundDevice::CreateBuffers ()
{
    DSBUFFERDESC        dsbd;
    WAVEFORMATEX        wfx;
    bool                primary_set = false;

    pMain->message( "creating DirectSound buffers..." );

    pMain->message( "...setting coop level to exclusive: " );
    if ( pDirectSound->SetCooperativeLevel( m_hWnd, DSSCL_EXCLUSIVE ) != DS_OK )
    {
        pMain->message( "failed" );
        return result::failure;
    }

    memset( &wfx, 0, sizeof(wfx) );
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.wBitsPerSample = 16;
    wfx.nSamplesPerSec = clamp<int>( snd_frequency, 11025, 44100 );
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    memset( &dsbd, 0, sizeof(DSBUFFERDESC) );
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat = 0;

    memset( &m_BufferCaps, 0, sizeof(DSBCAPS) );
    m_BufferCaps.dwSize = sizeof(DSBCAPS);

    pMain->message( "...creating primary buffer: " );
    if ( pDirectSound->CreateSoundBuffer( &dsbd, &pPrimaryBuffer, 0 ) == DS_OK )
    {

        m_BufferFormat = wfx;
        if ( pPrimaryBuffer->SetFormat( &m_BufferFormat ) != DS_OK )
            pMain->message( "...setting primary format: failed" );
        else
        {
            pMain->message( "...setting primary format: ok" );
            primary_set = true;
        }
    }
    else
        pMain->message( "failed" );

    if ( !primary_set || !snd_primary )
    {
        m_BufferFormat = wfx;

        memset( &dsbd, 0, sizeof(DSBUFFERDESC) );
        dsbd.dwSize = sizeof(DSBUFFERDESC);
        dsbd.dwFlags = DSBCAPS_CTRLFREQUENCY|DSBCAPS_LOCSOFTWARE;
        dsbd.dwBufferBytes = DEFAULT_BUFFER_SIZE;
        dsbd.lpwfxFormat = &m_BufferFormat;

        if ( snd_dsfocus )
            dsbd.dwFlags |= DSBCAPS_STICKYFOCUS;

        pMain->message( "...creating secondary buffer: " );
        if ( pDirectSound->CreateSoundBuffer( &dsbd, &pSoundBuffer, 0 ) != DS_OK )
        {
            pMain->message( "failed" );
            return result::failure;
        }

        if ( pSoundBuffer->GetCaps( &m_BufferCaps ) != DS_OK )
        {
            pMain->message( "...GetCaps failed" );
            return result::failure;
        }

        pMain->message( "...using secondary buffer" );
    }
    else
    {
        pMain->message( "...setting coop level to writeprimary: " );
        if ( pDirectSound->SetCooperativeLevel( m_hWnd, DSSCL_WRITEPRIMARY ) != DS_OK )
        {
            pMain->message( "failed\n" );
            return result::failure;
        }

        if ( pPrimaryBuffer->GetCaps( &m_BufferCaps ) != DS_OK )
        {
            pMain->message( "...GetCaps failed\n" );
            return result::failure;
        }

        pSoundBuffer = pPrimaryBuffer;
        pMain->message( "...using primary buffer" );
    }

    pMain->message( "output buffer format:" );
    pMain->message( "...channels:  %i", m_BufferFormat.nChannels );
    pMain->message( "...bit width: %i", m_BufferFormat.wBitsPerSample );
    pMain->message( "...frequency: %i", m_BufferFormat.nSamplesPerSec );

    pSoundBuffer->Play( 0, 0, DSBPLAY_LOOPING );
    m_nOffset = 0;

    return result::success;
}

/*=========================================================
=========================================================*/

void cDirectSoundDevice::DestroyBuffers ( )
{
    pMain->message( "destroying DirectSound buffers..." );

    if ( pDirectSound )
    {
        pMain->message( "...setting coop level to normal" );
        pDirectSound->SetCooperativeLevel( m_hWnd, DSSCL_NORMAL );
    }

    if ( pSoundBuffer && pSoundBuffer != pPrimaryBuffer )
    {
        pMain->message( "...releasing secondary buffer" );
        pSoundBuffer->Stop( );
        pSoundBuffer->Release( );
        
        if ( pPrimaryBuffer )
        {
            pMain->message( "...releasing primary buffer" );
            pPrimaryBuffer->Release( );
        }

        pPrimaryBuffer = NULL;
        pSoundBuffer = NULL;
    }
    else if ( pSoundBuffer )
    {
        pMain->message( "...releasing primary buffer" );

        pSoundBuffer->Stop( );
        pSoundBuffer->Release( );
        pPrimaryBuffer = NULL;
        pSoundBuffer = NULL;
    }
}

/*=========================================================
=========================================================*/

buffer_info_t cDirectSoundDevice::getBufferInfo ()
{
    buffer_info_t   info;

    info.channels = m_BufferFormat.nChannels;
    info.bitwidth = m_BufferFormat.wBitsPerSample;
    info.frequency = m_BufferFormat.nSamplesPerSec;
    info.size = m_BufferCaps.dwBufferBytes;
    
    pSoundBuffer->GetCurrentPosition( (LPDWORD )&info.read, (LPDWORD )&info.write );

    info.write = m_nOffset;

    return info;
}

/*=========================================================
=========================================================*/

void cDirectSoundDevice::writeToBuffer (byte *pAudioData, int nBytes)
{
    void                *pBuffer1, *pBuffer2;
    int                 nBytes1, nBytes2;
    int                 nSamples1, nSamples2;

    DWORD       dwStatus;

    pSoundBuffer->GetStatus( &dwStatus );
    if ( dwStatus & DSBSTATUS_BUFFERLOST )
        pSoundBuffer->Restore( );
    if ( !(dwStatus & DSBSTATUS_PLAYING) )
        pSoundBuffer->Play( 0, 0, DSBPLAY_LOOPING );
    
    pSoundBuffer->Lock( m_nOffset, nBytes, &pBuffer1, (LPDWORD )&nBytes1, &pBuffer2, (LPDWORD )&nBytes2, 0 );

    nSamples1 = nBytes1 / (m_BufferFormat.nChannels * m_BufferFormat.wBitsPerSample / 8);
    nSamples2 = nBytes2 / (m_BufferFormat.nChannels * m_BufferFormat.wBitsPerSample / 8);

    gSound->mixStereo16( (samplepair_t *)pAudioData, (stereo16_t *)pBuffer1, nSamples1, 255 );
    gSound->mixStereo16( (samplepair_t *)(pAudioData+nBytes1*2), (stereo16_t *)pBuffer2, nSamples2, 255 );

    pSoundBuffer->Unlock( pBuffer1, nBytes1, pBuffer2, nBytes2 );

    m_nOffset = (m_nOffset + nBytes1 + nBytes2)%m_BufferCaps.dwBufferBytes;
}
