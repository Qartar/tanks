//  cm_sound.h
//

#ifndef __CM_SOUND_H__
#define __CM_SOUND_H__

#ifndef _WINDOWS_
#ifndef __HWND__
#define __HWND__
struct HWND__ { int unused; };
typedef HWND__ *HWND;
#endif //__HWND__
#endif // _WINDOWS_

#define ATTN_STATIC     0.0f

#define MAX_SOUNDS      256
#define MAX_CHANNELS    64

typedef class vSoundChannel
{
public:
    virtual bool    isPlaying () = 0;
    virtual bool    isLooping () = 0;

    virtual int     playSound (int nSound) = 0;
    virtual int     playLoop (int nSound) = 0;

    virtual void    stopSound () = 0;   // stops loop also

    virtual void    setOrigin (vec3 vOrigin) = 0;
    virtual void    setVolume (float flVolume) = 0;
    virtual void    setFrequency (float flFreq) = 0;
    virtual void    setAttenuation (float flAttn) = 0;
} sndchan_t;

class vSound : public vObject
{
public:
    static void     Create ();
    static void     Destroy ();

    virtual void    onCreate (HWND hWnd) = 0;
    virtual void    onDestroy () = 0;

    virtual void    Update () = 0;

    virtual void    setListener (vec3 vOrigin, vec3 vForward, vec3 vRight, vec3 vUp) = 0;

    virtual void    playSound (int nIndex, vec3 vOrigin, float flVolume, float flAttenuation) = 0;

    virtual sndchan_t   *allocChan () = 0;
    virtual void        freeChan (sndchan_t *pChan) = 0;

    virtual int     Register (char *szFilename) = 0;
};

extern vSound   *pSound;

#endif //__CM_SOUND_H__
