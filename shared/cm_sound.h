//  cm_sound.h
//

#pragma once

#ifndef _WINDOWS_
typedef struct HWND__ *HWND;
#endif // _WINDOWS_

#define ATTN_STATIC     0.0f

#define MAX_SOUNDS      256
#define MAX_CHANNELS    64

////////////////////////////////////////////////////////////////////////////////
namespace sound {

enum class asset
{
    invalid = 0
};

//------------------------------------------------------------------------------
class channel
{
public:
    virtual bool playing() = 0;
    virtual bool looping() = 0;

    virtual int play(sound::asset asset) = 0;
    virtual int loop(sound::asset asset) = 0;
    virtual void stop() = 0;

    virtual void set_origin(vec3 origin) = 0;
    virtual void set_volume(float volume) = 0;
    virtual void set_frequency(float frequency) = 0;
    virtual void set_attenuation(float attenuation) = 0;
};

//------------------------------------------------------------------------------
class system
{
public:
    static void create ();
    static void destroy ();

    virtual void on_create(HWND hWnd) = 0;
    virtual void on_destroy() = 0;

    virtual void update() = 0;

    virtual void set_listener(vec3 origin, vec3 forward, vec3 right, vec3 up) = 0;

    virtual void play(sound::asset asset, vec3 origin, float volume, float attenuation) = 0;

    virtual sound::channel* allocate_channel() = 0;
    virtual void free_channel(sound::channel *channel) = 0;

    virtual sound::asset load_sound (char const* filename) = 0;
};

} // namespace sound

extern sound::system* pSound;
