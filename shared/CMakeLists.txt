add_library(shared STATIC
    cm_config.cpp
    cm_config.h
    cm_sound.h
    oed_error.h
    oed_files.h
    oed_mem.h
    oed_shared.h
    oed_tools.h
    oed_types.h
    shared.h
)

target_include_directories(shared PUBLIC .)
