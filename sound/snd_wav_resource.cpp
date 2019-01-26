// snd_wav_resource.cpp
//

#include "snd_main.h"
#include "snd_wav_resource.h"
#include "../system/resource.h"

////////////////////////////////////////////////////////////////////////////////
static const struct { int iResource; string::literal szResource; } resources[] =
{
    { IDR_WAVE1, "assets/sound/cannon_impact.wav" },
    { IDR_WAVE2, "assets/sound/tank_explode.wav" },
    { IDR_WAVE3, "assets/sound/cannon_fire.wav" },
    { IDR_WAVE4, "assets/sound/tank_idle.wav" },
    { IDR_WAVE5, "assets/sound/tank_move.wav" },
    { IDR_WAVE6, "assets/sound/turret_move.wav" },
    { IDR_WAVE7, "assets/sound/blaster_fire.wav" },
    { IDR_WAVE8, "assets/sound/blaster_impact.wav" },
    { IDR_WAVE9, "assets/sound/missile_flight.wav" },
};

//------------------------------------------------------------------------------
result cSoundWaveResource::load(string::view filename)
{
    char const* resourcename = nullptr;
    for (int i = 0; i < countof(resources); i++) {
        if (strcmp(filename, resources[i].szResource) == 0) {
            resourcename = MAKEINTRESOURCE(resources[i].iResource);
            break;
        }
    }

    if (!resourcename) {
        return result::failure;
    }

    HMODULE hinstance = GetModuleHandleA(NULL);
    HANDLE hinfo = FindResourceA(hinstance, resourcename, "WAVE");
    DWORD size = SizeofResource(hinstance, (HRSRC)hinfo);
    HANDLE resource = LoadResource(hinstance, (HRSRC)hinfo);
    LPVOID resource_data = LockResource(resource);

    // read wave data
    {
        chunk_file reader((byte *)resource_data, size);

        while (reader.id()) {
            parse_chunk(reader);
            reader.next();
        }
    }

    UnlockResource(resource_data);
    FreeResource(resource);

    return (_num_samples > 0 ? result::success : result::failure);
}
