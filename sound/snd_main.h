/*=========================================================
Name    :   snd_main.h
Date    :   04/01/2006
=========================================================*/

#pragma once

#define NOMINMAX
#include <windows.h>

#include "shared.h"

#include "cm_config.h"
#include "cm_sound.h"

#include "snd_device.h"
#include "snd_files.h"

/*=========================================================
=========================================================*/

typedef struct samplepair_s
{
    int     left;
    int     right;
} samplepair_t;

typedef struct stereo16_s
{
    short   left;
    short   right;
} stereo16_t;

typedef struct stereo8_s
{
    byte    left;
    byte    right;
} stereo8_t;

/*=========================================================
=========================================================*/

typedef struct snd_link_s
{
    snd_link_s  *pNext, *pPrev;
    int         nSequence;      // registration sequence
    char    szFilename[SHORT_STRING];
    int     nNumber;

    cSoundSource    *pSource;
} snd_link_t;

#define PAINTBUFFER_BYTES   4

typedef struct paintbuffer_s
{
    int     nFrequency;
    int     nChannels;
    int     nVolume;
    int     nSize;
    byte    *pData;
} paintbuffer_t;

/*=========================================================
=========================================================*/

class cSoundChannel : public sound::channel
{
public:
    cSoundChannel () { m_flFrequency = m_flVolume = 1.0f; }

    virtual bool        playing () { return m_bPlaying; }
    virtual bool        looping () { return m_bLooping; }

    virtual int         play (sound::asset asset, bool bLooping);
    virtual int         play (sound::asset asset);
    virtual int         loop (sound::asset asset);

    virtual void        stop ();

    virtual void        set_origin (vec3 vOrigin) { m_vOrigin = vOrigin; }
    virtual void        set_volume (float flVolume) { m_flVolume = flVolume; }
    virtual void        set_frequency (float flFreq) { m_flFrequency = flFreq; }
    virtual void        set_attenuation (float flAttn) { m_flAttenuation = flAttn; }

    void        setReserved (bool b) { m_bReserved = b; }
    bool        isReserved () { return m_bReserved; }

    void        mixChannel (paintbuffer_t *pBuffer, int nSamples);
private:
    cSoundSource    *m_pSound;

    vec3        m_vOrigin;
    
    bool        m_bPlaying;
    bool        m_bLooping;
    float       m_flAttenuation;
    float       m_flFrequency;
    float       m_flVolume;

    float       m_nSamplePos;
    bool        m_bReserved;

    void        m_mixMono16 (void *pBuffer, float flRate, int nVolume, int nSamples);
    void        m_mixStereo16 (void *pBuffer, float flRate, int nVolume, int nSamples);

    void        m_SpatializeMono (int in, int *out);
    void        m_SpatializeStereo (int in, int out[2]);
};

/*=========================================================
=========================================================*/

class cSound : public sound::system
{
public:
    cSound ();
    ~cSound () { Shutdown( ); }

    int     Init ();
    int     Shutdown ();

    virtual void    on_create (HWND hWnd);
    virtual void    on_destroy ();

    virtual void    update ();

    virtual void    set_listener (vec3 vOrigin, vec3 vForward, vec3 vRight, vec3 vUp);

    virtual void    play (sound::asset asset, vec3 vOrigin, float flVolume, float flAttenuation);

    virtual sound::channel   *allocate_channel () { return m_allocChan( true ); }
    virtual void        free_channel (sound::channel *pChan);

    //  memory

    void        *heapAlloc (unsigned int size) { return alloc(size); }
    void        heapFree (void *ptr) { free(ptr); }

    //  registration

    sound::asset    load_sound (char const *szFilename);
    cSoundSource    *getSound (int nSound) { if (m_Sounds[nSound]) return m_Sounds[nSound]->pSource; return NULL; }

    //  mixing

    void            mixStereo16 (samplepair_t *pInput, stereo16_t *pOutput, int nSamples, int nVolume);
    paintbuffer_t   *getChannelBuffer () { return &m_channelBuffer; }

    //  spatialization

    vec3    vOrigin, vForward, vRight, vUp;

private:
    config::boolean snd_disable;
    config::scalar snd_volume;
    config::integer snd_frequency;
    config::scalar snd_mixahead;
    config::boolean snd_primary;

    bool        m_bInitialized;

    cAudioDevice    *pAudioDevice;
    
    paintbuffer_t       *getPaintBuffer (int nBytes);
    paintbuffer_t       m_paintBuffer;
    paintbuffer_t       m_channelBuffer;

    //
    //  memory
    //

    HANDLE      m_hHeap;
    void        *alloc (unsigned int size);
    void        free (void *ptr);

    //
    //  sounds
    //

    snd_link_t  m_Chain;
    snd_link_t  *m_Sounds[MAX_SOUNDS];

    snd_link_t      *Create (char const *szFilename);
    snd_link_t      *Find (char const *szFilename);
    void            Delete (snd_link_t *pLink);

    //
    //  channels
    //

    sound::channel       *m_allocChan (bool bReserve);

    void            m_mixChannels (paintbuffer_t *pBuffer, int nSamples);
    cSoundChannel   m_Channels[MAX_CHANNELS];
};

extern cSound   *gSound;
