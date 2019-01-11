// snd_channel.cpp
//

#include "snd_main.h"

//------------------------------------------------------------------------------
sound::channel *cSound::alloc_channel(bool reserve)
{
    if (!_audio_device) {
        return nullptr;
    }

    for (int ii = 0; ii < MAX_CHANNELS; ++ii) {
        if (_channels[ii].playing()) {
            continue;
        }

        if (_channels[ii].is_reserved()) {
            continue;
        }

        if (reserve) {
            _channels[ii].set_reserved(true);
        }

        return (sound::channel *)&_channels[ii];
    }

    return nullptr;
}

//------------------------------------------------------------------------------
void cSound::free_channel(sound::channel* chan)
{
    cSoundChannel* channel = (cSoundChannel *)chan;

    channel->stop();
    channel->set_reserved(false);
}

//------------------------------------------------------------------------------
void cSound::mix_channels(paintbuffer_t* buffer, int num_samples)
{
    for (int ii = 0; ii < MAX_CHANNELS; ++ii) {
        if (_channels[ii].playing()) {
            _channels[ii].mix(buffer, num_samples);
        }
    }
}

//------------------------------------------------------------------------------
result cSoundChannel::play(sound::asset asset, bool looping)
{
    if (asset == sound::asset::invalid) {
        return result::failure;
    }

    _sound = gSound->get_sound(narrow_cast<int>(asset) - 1);

    if (!_sound) {
        log::warning("could not play sound %i: does not exist\n", static_cast<int>(asset) - 1);
        return result::failure;
    }

    _sample_pos = 0;
    _is_playing = true;
    _is_looping = looping;

    return result::success;
}

//------------------------------------------------------------------------------
result cSoundChannel::play(sound::asset asset)
{
    return play(asset, false);
}

//------------------------------------------------------------------------------
result cSoundChannel::loop(sound::asset asset)
{
    return play(asset, true);
}

//------------------------------------------------------------------------------
void cSoundChannel::stop()
{
    _is_playing = false;
    _is_looping = false;
}

//------------------------------------------------------------------------------
void cSoundChannel::mix(paintbuffer_t* buffer, int samples)
{
    int     volume = (int)(_volume * buffer->volume * 255) >> 8;

    sound_format const* format = _sound->get_format();

    float   rate = _frequency * (float)format->frequency / (float)buffer->frequency;

    if (format->channels == 1 && format->bitwidth == 16) {
        _mix_mono16(buffer->data, rate, volume, samples);
    }
    if (format->channels == 2 && format->bitwidth == 16) {
        _mix_stereo16(buffer->data, rate, volume, samples);
    }
}

//------------------------------------------------------------------------------
#define SAMPLE(a,b,c)   ( (float)(a[b]*(1-c))+(float)(a[b+1]*(c)) )

void cSoundChannel::_mix_mono16(void* buffer, float rate, int volume, int num_samples)
{
    samplepair_t* output = (samplepair_t *)buffer;
    short* input = (short *)gSound->get_channel_buffer()->data;

    int spatial_vol[2];

    _spatialize_stereo(volume, spatial_vol);

    std::size_t samples_read = _sound->get_samples((byte *)input, num_samples * rate, _sample_pos, _is_looping);
    float sample_pos = _sample_pos - floor(_sample_pos);

    for (int ii = 0; sample_pos < samples_read; ++ii) {
        int sample_index = floor(sample_pos);
        float sample_frac = sample_pos - sample_index;

        output[ii].left += (int)(spatial_vol[0] * SAMPLE(input, sample_index, sample_frac)) >> 8;
        output[ii].right += (int)(spatial_vol[1] * SAMPLE(input, sample_index, sample_frac)) >> 8;

        _sample_pos += rate;
        sample_pos += rate;
    }

    if (samples_read < (int)num_samples * rate) {
        _is_playing = false;
    }

    _sample_pos = _sound->get_position(_sample_pos);
}

#undef SAMPLE

//------------------------------------------------------------------------------
#define SAMPLE(a,b,c,d) ( (float)(a[c].b*(1-d))+(float)(a[c+1].b*(d)) )

void cSoundChannel::_mix_stereo16(void* buffer, float rate, int volume, int num_samples)
{
    samplepair_t* output = (samplepair_t *)buffer;
    stereo16_t* input = (stereo16_t *)gSound->get_channel_buffer()->data;

    int spatial_vol[2];

    _spatialize_stereo(volume, spatial_vol);

    std::size_t samples_read = _sound->get_samples((byte *)input, num_samples * rate, _sample_pos, _is_looping);
    float sample_pos = _sample_pos - floor(_sample_pos);

    for (int ii = 0; sample_pos < samples_read; ++ii) {
        int sample_index = floor(sample_pos);
        float sample_frac = sample_pos - sample_index;

        output[ii].left += (int)(spatial_vol[0] * SAMPLE(input, left, sample_index, sample_frac)) >> 8;
        output[ii].right += (int)(spatial_vol[1] * SAMPLE(input, right, sample_index, sample_frac)) >> 8;

        _sample_pos += rate;
        sample_pos += rate;
    }

    if (samples_read < (int)num_samples * rate) {
        _is_playing = false;
    }

    _sample_pos = _sound->get_position(_sample_pos);
}

#undef SAMPLE

//------------------------------------------------------------------------------
#define ATTN_LEN        1000.0f

void cSoundChannel::_spatialize_mono(int in, int *out)
{
    if (_attenuation == ATTN_STATIC) {
        out[0] = in;
    } else {
        vec3    dir = _origin - gSound->_origin;
        dir.normalize_self();

        float   attn = clamp(powf(ATTN_LEN / dir.length_sqr(), _attenuation), 0.0f, 1.0f);

        out[0] = in * attn;
    }
}

//------------------------------------------------------------------------------
void cSoundChannel::_spatialize_stereo(int in, int out[2])
{
    vec3 dir = _origin - gSound->_origin;
    float dist = dir.normalize_length();
    float dp = dir.dot(gSound->_axis[1]);

    if (_attenuation == ATTN_STATIC) {
        out[0] = in * 0.5f * (1.0f - dp);
        out[1] = in * 0.5f * (1.0f + dp);
    } else {
        float   attn = clamp(powf(ATTN_LEN / dist, _attenuation), 0.0f, 1.0f);

        out[0] = in * 0.5f * (1 - dp) * attn;
        out[1] = in * 0.5f * (1 + dp) * attn;
    }
}
