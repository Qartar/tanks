/*=========================================================
Name    :   snd_main.h
Date    :   04/01/2006
=========================================================*/

#include <windows.h>

#include "../shared.h"

#include "../cm_sound.h"
#include "../cm_variable.h"

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

class cSoundChannel : public vSoundChannel
{
public:
    cSoundChannel () { m_flFrequency = m_flVolume = 1.0f; }

    virtual bool        isPlaying () { return m_bPlaying; }
    virtual bool        isLooping () { return m_bLooping; }

    virtual int         playSound (int nSound, bool bLooping);
    virtual int         playSound (int nSound);
    virtual int         playLoop (int nSound);

    virtual void        stopSound ();

    virtual void        setOrigin (vec3 vOrigin) { m_vOrigin = vOrigin; }
    virtual void        setVolume (float flVolume) { m_flVolume = flVolume; }
    virtual void        setFrequency (float flFreq) { m_flFrequency = flFreq; }
    virtual void        setAttenuation (float flAttn) { m_flAttenuation = flAttn; }

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

class cSound : public vSound
{
public:
    cSound () { m_Chain.pNext = m_Chain.pPrev = &m_Chain; Init( ); }
    ~cSound () { Shutdown( ); }

    int     Init ();
    int     Shutdown ();

    virtual void    onCreate (HWND hWnd);
    virtual void    onDestroy ();

    virtual void    Update ();

    virtual void    setListener (vec3 vOrigin, vec3 vForward, vec3 vRight, vec3 vUp);

    virtual void    playSound (int nIndex, vec3 vOrigin, float flVolume, float flAttenuation);

    virtual sndchan_t   *allocChan () { return m_allocChan( true ); }
    virtual void        freeChan (sndchan_t *pChan);

    //  memory

    void        *heapAlloc (unsigned int size) { return alloc(size); }
    void        heapFree (void *ptr) { free(ptr); }

    //  registration

    int             Register (char *szFilename);
    cSoundSource    *getSound (int nSound) { if (m_Sounds[nSound]) return m_Sounds[nSound]->pSource; return NULL; }

    //  mixing

    void            mixStereo16 (samplepair_t *pInput, stereo16_t *pOutput, int nSamples, int nVolume);
    paintbuffer_t   *getChannelBuffer () { return &m_channelBuffer; }

    //  spatialization

    vec3    vOrigin, vForward, vRight, vUp;

private:
    cvar_t  *snd_disable;
    cvar_t  *snd_volume;
    cvar_t  *snd_frequency;
    cvar_t  *snd_mixahead;
    cvar_t  *snd_primary;

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

    snd_link_t      *Create (char *szFilename);
    snd_link_t      *Find (char *szFilename);
    void            Delete (snd_link_t *pLink);

    //
    //  channels
    //

    sndchan_t       *m_allocChan (bool bReserve);

    void            m_mixChannels (paintbuffer_t *pBuffer, int nSamples);
    cSoundChannel   m_Channels[MAX_CHANNELS];
};

extern cSound   *gSound;
