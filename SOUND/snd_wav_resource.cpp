/*=========================================================
Name    :   snd_wav_resource.cpp
Date    :   10/14/2016
=========================================================*/

#include "snd_main.h"
#include "snd_wav_resource.h"
#include "../resource.h"

/*=========================================================
=========================================================*/

static const struct { int iResource; char const* szResource; } resources[] =
{
    { IDR_WAVE1, "ASSETS\\SOUND\\BULLET_EXPLODE.wav" },
    { IDR_WAVE2, "ASSETS\\SOUND\\TANK_EXPLODE.wav" },
    { IDR_WAVE3, "ASSETS\\SOUND\\TANK_FIRE.wav" },
    { IDR_WAVE4, "ASSETS\\SOUND\\TANK_IDLE.wav" },
    { IDR_WAVE5, "ASSETS\\SOUND\\TANK_MOVE.wav" },
    { IDR_WAVE6, "ASSETS\\SOUND\\TURRET_MOVE.wav" },
};

int cSoundWaveResource::Load (char *szFilename)
{
    for ( int i = 0; i < _countof(resources); i++ ) {
        if ( strcmp(szFilename, resources[i].szResource) == 0 ) {
            szFilename = MAKEINTRESOURCE( resources[i].iResource );
            break;
        }
    }

    HMODULE hInstance = GetModuleHandle( NULL );
    HANDLE hInfo = FindResource( hInstance, szFilename, "WAVE" );
    DWORD dwSize = SizeofResource( hInstance, (HRSRC )hInfo );
    m_hResource = LoadResource( hInstance, (HRSRC )hInfo );
    m_lpvData = LockResource( m_hResource );

    // read wave data
    {
        riffChunk_t reader = riffChunk_t( (byte *)m_lpvData, dwSize );

        while ( reader.getName( ) )
        {
            parseChunk( reader );
            reader.chunkNext( );
        }

        reader.chunkClose( );
    }

    UnlockResource( m_lpvData );
    FreeResource( m_hResource );
    
    return (m_numSamples > 0 ? ERROR_NONE : ERROR_FAIL);
}
