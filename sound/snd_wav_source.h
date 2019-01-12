// snd_wav_source.h
//

#pragma once

#include "snd_files.h"
#include "cm_filesystem.h"

////////////////////////////////////////////////////////////////////////////////
#define STREAM_THRESHOLD    0x10000     // 65k

constexpr int make_id(char a, char b, char c, char d)
{
    return (int(d) << 24) | (int(c) << 16) | (int(b) << 8) | (int(a));
}

constexpr int CHUNK_FMT     = make_id('f','m','t',' ');
constexpr int CHUNK_CUE     = make_id('c','u','e',' ');
constexpr int CHUNK_DATA    = make_id('d','a','t','a');

//------------------------------------------------------------------------------
class chunk_file
{
public:
    chunk_file(char const* filename);
    chunk_file(byte const* buffer, std::size_t buffer_size);

    bool next();

    uint32_t id() const;
    std::size_t size() const;

    std::size_t tell() const;
    std::size_t seek(std::size_t pos);

    std::size_t read(byte* buffer, std::size_t buffer_size);
    int read_int();

protected:
    bool read_chunk();

    file::stream _stream;
    byte const* _buffer;
    std::size_t _buffer_size;

    std::size_t _start;
    std::size_t _size;
    uint32_t _id;
    std::size_t _pos;

    uint32_t _chunk_id;
    std::size_t _chunk_size;
    std::size_t _chunk_start;
};

//------------------------------------------------------------------------------
class cSoundWaveSource : public cSoundSource
{
public:
    virtual ~cSoundWaveSource() = 0 {};

    virtual std::size_t get_samples(byte* samples, int num_samples, int sample_offset, bool looping) override = 0;
    virtual sound_format const* get_format() const override { return &_format; }
    virtual char const* get_filename() const override { return _filename.c_str(); }
    virtual float get_position(float position) const override;

    virtual result load(char const* filename) override = 0;
    virtual void free() override = 0;

protected:
    bool parse_chunk(chunk_file& chunk);

    virtual bool parse_format(chunk_file& chunk);
    virtual bool parse_cue(chunk_file& chunk);
    virtual bool parse_data(chunk_file& chunk) = 0;

    sound_format _format;
    std::string _filename;

    std::size_t _num_samples;
    int _loop_start;
};
