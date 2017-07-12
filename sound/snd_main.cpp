/*=========================================================
Name    :   snd_main.cpp
Date    :   04/01/2006
=========================================================*/

#include "snd_main.h"

cSound  *gSound;

sound::system* pSound = nullptr;

void sound::system::create()
{
    pSound = new cSound;
}

void sound::system::destroy()
{
    delete pSound;
    pSound = nullptr;
}

/*=========================================================
=========================================================*/

cSound::cSound()
    : snd_disable("snd_disable", false, config::archive, "disables sound playback")
    , snd_volume("snd_volume", 0.5f, config::archive, "sound volume")
    , snd_frequency("snd_frequency", 22050, config::archive, "sound playback speed")
    , snd_mixahead("snd_mixahead", 0.1f, config::archive, "sound mix ahead time, in seconds")
    , snd_primary("snd_primary", false, config::archive, "use primary sound buffer")
{
    m_Chain.pNext = m_Chain.pPrev = &m_Chain; Init( );
}

int cSound::Init ()
{
    gSound = this;

    m_bInitialized = false;

    m_paintBuffer.pData = NULL;
    m_paintBuffer.nSize = 0;

    m_channelBuffer.pData = NULL;
    m_channelBuffer.nSize = 0;

    memset( m_Sounds, 0, sizeof(m_Sounds) );
    for ( int i=0 ; i<MAX_CHANNELS ; i++ )
    {
        m_Channels[i].setReserved( false );
        m_Channels[i].stop( );
    }

    if ( !snd_disable )
    {
        SYSTEM_INFO sysinfo;

        GetSystemInfo( &sysinfo );

        if ( (m_hHeap = HeapCreate( 0, sysinfo.dwPageSize, 0 )) == NULL )
            pMain->message( "sound heap allocation failed\n" );
    }
    else
        m_hHeap = false;

    return ERROR_NONE;
}

int cSound::Shutdown ()
{
    if ( m_hHeap )
    {
        HeapDestroy( m_hHeap );
        m_hHeap = NULL;
    }

    return ERROR_NONE;
}

/*=========================================================
=========================================================*/

void cSound::on_create (HWND hWnd)
{
    if ( snd_disable )
        return;

    pAudioDevice = cAudioDevice::Create( hWnd );
}

void cSound::on_destroy ()
{
    // clear sound chain
    while ( m_Chain.pNext != &m_Chain )
        Delete( m_Chain.pNext );

    if ( m_paintBuffer.pData )
    {
        free( m_paintBuffer.pData );
        free( m_channelBuffer.pData );

        m_paintBuffer.pData = NULL;
        m_paintBuffer.nSize = 0;

        m_channelBuffer.pData = NULL;
        m_channelBuffer.nSize = 0;
    }

    for ( int i=0 ; i<MAX_CHANNELS ; i++ )
        free_channel( m_Channels + i );

    cAudioDevice::Destroy( pAudioDevice );
    pAudioDevice = NULL;
}

/*=========================================================
=========================================================*/

void cSound::update ()
{
    paintbuffer_t   *pBuffer;
    int             nBytes;
    buffer_info_t   info;
    int             nSamples, nWritten;

    if ( !pAudioDevice )
        return;

    info = pAudioDevice->getBufferInfo( );

    nSamples = snd_mixahead * info.frequency;
    pBuffer = getPaintBuffer( nSamples * info.channels * PAINTBUFFER_BYTES );

    pBuffer->nFrequency = info.frequency;
    pBuffer->nChannels = info.channels;
    pBuffer->nVolume = snd_volume * 255;

    nWritten = info.write - info.read;
    if ( nWritten < 0 )
        nWritten += info.size;

    if ( nSamples > info.size )
        nSamples = info.size;

    nSamples -= nWritten / (info.channels * info.bitwidth / 8);

    if ( nSamples < 0 )
        return;

    m_mixChannels( pBuffer, nSamples );

    nBytes = nSamples * info.channels * info.bitwidth / 8;

    pAudioDevice->writeToBuffer( pBuffer->pData, nBytes );
}

paintbuffer_t *cSound::getPaintBuffer (int nBytes)
{
    if ( !m_paintBuffer.pData )
    {
        m_paintBuffer.pData = (byte *)alloc( nBytes );
        m_paintBuffer.nSize = nBytes;

        m_channelBuffer.pData = (byte *)alloc( nBytes );
        m_channelBuffer.nSize = nBytes;
    }
    else if ( nBytes != m_paintBuffer.nSize )
    {
        free( m_paintBuffer.pData );
        free( m_channelBuffer.pData );

        m_paintBuffer.pData = (byte *)alloc( nBytes );
        m_paintBuffer.nSize = nBytes;

        m_channelBuffer.pData = (byte *)alloc( nBytes );
        m_channelBuffer.nSize = nBytes;
    }

    memset( m_paintBuffer.pData, 0, m_paintBuffer.nSize );
    return &m_paintBuffer;
}

/*=========================================================
=========================================================*/

void cSound::set_listener (vec3 vOrigin, vec3 vForward, vec3 vRight, vec3 vUp)
{
    this->vOrigin = vOrigin;
    this->vForward = vForward;
    this->vRight = vRight;
    this->vUp = vUp;
}

/*=========================================================
=========================================================*/

void *cSound::alloc (unsigned int size)
{
    void    *ptr;

    if ( !m_hHeap || (ptr = HeapAlloc( m_hHeap, 0, size )) == NULL)
        return mem::alloc( size );
    return ptr;
}

void cSound::free (void *ptr)
{
    if ( m_hHeap )
        HeapFree( m_hHeap, 0, ptr );
    else
        mem::free( ptr );
}

/*=========================================================
=========================================================*/

snd_link_t *cSound::Create (char const *szFilename)
{
    int         i, len = strlen(szFilename);
    snd_link_t      *pLink, *next;
    cSoundSource    *pSource;

    pSource = cSoundSource::createSound( szFilename );
    if ( !pSource )
        return NULL;

    pLink = (snd_link_t *)alloc( sizeof(snd_link_t) );

    // alphabetize
    for ( next = m_Chain.pNext ; (next != &m_Chain) && (stricmp(next->szFilename,szFilename)<0) ; next = next->pNext );

    pLink->pNext = next;
    pLink->pPrev = next->pPrev;

    pLink->pNext->pPrev = pLink;
    pLink->pPrev->pNext = pLink;

    for ( i = 0 ; i<MAX_SOUNDS ; i++ )
    {
        if ( m_Sounds[i] == NULL )
        {
            m_Sounds[i] = pLink;
            pLink->nNumber = i;
            break;
        }
    }
    if ( i == MAX_SOUNDS )
    {
        pLink->nNumber = MAX_SOUNDS;
        Delete( pLink );

        pMain->message( "could not load %s: out of room\n", szFilename );
        return NULL;
    }


    strcpy( pLink->szFilename, szFilename );
    pLink->nSequence = 0;

    pLink->pSource = pSource;

    return pLink;
}

/*=========================================================
=========================================================*/

snd_link_t *cSound::Find (char const *szFilename)
{
    snd_link_t  *pLink;
    int         cmp;

    for ( pLink = m_Chain.pNext ; pLink != &m_Chain ; pLink = pLink->pNext )
    {
        cmp = strcmp( szFilename, pLink->szFilename );
        if ( cmp == 0 )
            return pLink;
        else if ( cmp < 0 ) // passed it, does not exit
            return NULL;
    }
    return NULL;
}

/*=========================================================
=========================================================*/

void cSound::Delete (snd_link_t *pLink)
{
    pLink->pNext->pPrev = pLink->pPrev;
    pLink->pPrev->pNext = pLink->pNext;

    if ( pLink->nNumber < MAX_SOUNDS )
        m_Sounds[pLink->nNumber] = NULL;

    cSoundSource::destroySound( pLink->pSource );

    free( pLink );
}

/*=========================================================
=========================================================*/

int cSound::load_sound (char const *szFilename)
{
    snd_link_t  *pLink;

    if ( pLink = Find( szFilename ) )
        pLink->nSequence = 0;
    else
        pLink = Create( szFilename );

    if ( pLink )
        return pLink->nNumber;
    return -1;
}

/*=========================================================
=========================================================*/

void cSound::play (int nIndex, vec3 vOrigin, float flVolume, float flAttenuation)
{
    sound::channel   *pChannel = m_allocChan( false );

    if ( pChannel )
    {
        pChannel->set_volume( flVolume );
        pChannel->set_origin( vOrigin );
        pChannel->set_frequency( 1.0f );
        pChannel->set_attenuation( flAttenuation );
        
        pChannel->play( nIndex );
    }
}
