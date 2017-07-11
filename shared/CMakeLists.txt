add_library(shared STATIC
    cm_sound.h
    cm_variable.cpp
    cm_variable.h
    oed_error.h
    oed_files.h
    oed_mem.h
    oed_shared.h
    oed_tools.h
    oed_types.h
    shared.h
)

target_include_directories(shared PUBLIC .)
