set(SHARED_SOURCES
    cm_bounds.h
    cm_color.h
    cm_config.cpp
    cm_config.h
    cm_matrix.h
    cm_sound.h
    cm_vector.h
    oed_error.h
    oed_files.h
    oed_mem.h
    oed_shared.cpp
    oed_shared.h
    oed_tools.h
    shared.h
)

add_library(shared STATIC ${SHARED_SOURCES})
target_include_directories(shared PUBLIC .)
source_group("\\" FILES ${SHARED_SOURCES})
