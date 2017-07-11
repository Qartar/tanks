add_library(sound STATIC
    snd_channel.cpp
    snd_device.cpp
    snd_device.h
    snd_dsound.cpp
    snd_dsound.h
    snd_files.h
    snd_main.cpp
    snd_main.h
    snd_mix.cpp
    snd_wav_cache.cpp
    snd_wav_cache.h
    snd_wav_reader.cpp
    snd_wav_source.cpp
    snd_wav_source.h
    snd_wav_stream.cpp
    snd_wav_stream.h
    snd_wav_resource.cpp
    snd_wav_resource.h
)

target_link_libraries(sound PUBLIC shared)
