// snd_main.cpp
//

#include "snd_main.h"

////////////////////////////////////////////////////////////////////////////////
cSound  *gSound;

sound::system* pSound = nullptr;

//------------------------------------------------------------------------------
void sound::system::create()
{
    pSound = new cSound;
}

//------------------------------------------------------------------------------
void sound::system::destroy()
{
    delete pSound;
    pSound = nullptr;
}

//------------------------------------------------------------------------------
cSound::cSound()
    : snd_disable("snd_disable", false, config::archive, "disables sound playback")
    , snd_volume("snd_volume", 0.5f, config::archive, "sound volume")
    , snd_frequency("snd_frequency", 22050, config::archive, "sound playback speed")
    , snd_mixahead("snd_mixahead", 0.1f, config::archive, "sound mix ahead time, in seconds")
    , snd_primary("snd_primary", false, config::archive, "use primary sound buffer")
    , _initialized(false)
    , _audio_device(nullptr)
{
    _chain.next = _chain.prev = &_chain; init();
}

//------------------------------------------------------------------------------
result cSound::init()
{
    gSound = this;

    _initialized = false;

    _paint_buffer.data = nullptr;
    _paint_buffer.size = 0;

    _channel_buffer.data = nullptr;
    _channel_buffer.size = 0;

    memset(_sounds, 0, sizeof(_sounds));
    for (int ii = 0; ii < MAX_CHANNELS; ++ii) {
        _channels[ii].set_reserved(false);
        _channels[ii].stop();
    }

    return result::success;
}

//------------------------------------------------------------------------------
result cSound::shutdown()
{
    return result::success;
}

//------------------------------------------------------------------------------
void cSound::on_create(HWND hwnd)
{
    if (snd_disable) {
        return;
    }

    _audio_device = cAudioDevice::create(hwnd);
}

//------------------------------------------------------------------------------
void cSound::on_destroy()
{
    // clear sound chain
    while (_chain.next != &_chain) {
        free(_chain.next);
    }

    if (_paint_buffer.data) {
        delete [] _paint_buffer.data;
        delete [] _channel_buffer.data;

        _paint_buffer.data = NULL;
        _paint_buffer.size = 0;

        _channel_buffer.data = NULL;
        _channel_buffer.size = 0;
    }

    for (int ii = 0; ii < MAX_CHANNELS; ++ii) {
        free_channel(_channels + ii);
    }

    cAudioDevice::destroy(_audio_device);
    _audio_device = NULL;
}

//------------------------------------------------------------------------------
void cSound::update()
{
    if (!_audio_device) {
        return;
    }

    buffer_info_t info = _audio_device->get_buffer_info();
    int num_samples = snd_mixahead * info.frequency;
    paintbuffer_t* buffer = get_paint_buffer(num_samples * info.channels * PAINTBUFFER_BYTES);

    buffer->frequency = info.frequency;
    buffer->channels = info.channels;
    buffer->volume = snd_volume * 255;

    int num_written = info.write - info.read;
    if (num_written < 0) {
        num_written += info.size;
    }

    if (num_samples > info.size) {
        num_samples = info.size;
    }

    num_samples -= num_written / (info.channels * info.bitwidth / 8);

    if (num_samples < 0) {
        return;
    }

    mix_channels(buffer, num_samples);

    int num_bytes = num_samples * info.channels * info.bitwidth / 8;

    _audio_device->write(buffer->data, num_bytes);
}

//------------------------------------------------------------------------------
void cSound::mix_stereo16(samplepair_t* input, stereo16_t* output, int num_samples, int volume)
{
    for (int ii = 0; ii < num_samples; ++ii) {
        int left = (input[ii].left * volume) >> 8;
        output[ii].left = clamp<int>(left, INT16_MIN, INT16_MAX);

        int right = (input[ii].right * volume) >> 8;
        output[ii].right = clamp<int>(right, INT16_MIN, INT16_MAX);
    }
}

//------------------------------------------------------------------------------
paintbuffer_t* cSound::get_paint_buffer(int num_bytes)
{
    if (!_paint_buffer.data) {
        _paint_buffer.data = new byte[num_bytes];
        _paint_buffer.size = num_bytes;

        _channel_buffer.data = new byte[num_bytes];
        _channel_buffer.size = num_bytes;
    } else if (num_bytes != _paint_buffer.size) {
        delete [] _paint_buffer.data;
        delete [] _channel_buffer.data;

        _paint_buffer.data = new byte[num_bytes];
        _paint_buffer.size = num_bytes;

        _channel_buffer.data = new byte[num_bytes];
        _channel_buffer.size = num_bytes;
    }

    memset(_paint_buffer.data, 0, _paint_buffer.size);
    return &_paint_buffer;
}

//------------------------------------------------------------------------------
void cSound::set_listener(vec3 origin, vec3 forward, vec3 right, vec3 up)
{
    _origin = origin;
    _axis = mat3(forward, right, up);
}

//------------------------------------------------------------------------------
snd_link_t *cSound::create(char const* filename)
{
    snd_link_t* link = nullptr;
    snd_link_s* next = nullptr;
    cSoundSource* source;

    source = cSoundSource::create(filename);
    if (!source) {
        return nullptr;
    }

    link = new snd_link_t;

    // alphabetize
    for (next = _chain.next; (next != &_chain) && (_stricmp(next->filename.c_str(), filename) < 0); next = next->next);

    link->next = next;
    link->prev = next->prev;

    link->next->prev = link;
    link->prev->next = link;

    for (int ii = 0; ii < MAX_SOUNDS; ++ii) {
        if (_sounds[ii] == nullptr) {
            _sounds[ii] = link;
            link->number = ii;

            link->filename = filename;
            link->sequence = 0;
            link->source = source;

            return link;
        }
    }

    link->number = MAX_SOUNDS;
    free(link);

    log::message("could not load %s: out of room\n", filename);
    return nullptr;
}

//---------------------------------- --------------------------------------------
snd_link_t *cSound::find(char const* filename)
{
    for (snd_link_t* link = _chain.next; link != &_chain; link = link->next) {
        int cmp = strcmp(filename, link->filename.c_str());
        if (cmp == 0) {
            return link;
        } else if (cmp < 0) { // passed it, does not exit
            return NULL;
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
void cSound::free(snd_link_t* link)
{
    link->next->prev = link->prev;
    link->prev->next = link->next;

    if (link->number < MAX_SOUNDS) {
        _sounds[link->number] = NULL;
    }

    cSoundSource::destroy(link->source);

    delete link;
}

//------------------------------------------------------------------------------
sound::asset cSound::load_sound(char const* filename)
{
    snd_link_t* link;

    if (link = find(filename)) {
        link->sequence = 0;
    } else {
        link = create(filename);
    }

    if (link) {
        return static_cast<sound::asset>(link->number + 1);
    }
    return sound::asset::invalid;
}

//------------------------------------------------------------------------------
void cSound::play(sound::asset asset, vec3 origin, float volume, float attenuation)
{
    sound::channel* ch = alloc_channel(false);

    if (ch) {
        ch->set_volume(volume);
        ch->set_origin(origin);
        ch->set_frequency(1.0f);
        ch->set_attenuation(attenuation);

        ch->play(asset);
    }
}
