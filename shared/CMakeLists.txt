set(SHARED_SOURCES
    cm_bounds.h
    cm_color.h
    cm_config.cpp
    cm_config.h
    cm_error.h
    cm_filesystem.cpp
    cm_filesystem.h
    cm_matrix.h
    cm_parser.cpp
    cm_parser.h
    cm_shared.cpp
    cm_shared.h
    cm_sound.h
    cm_time.h
    cm_vector.h
)

add_library(shared STATIC ${SHARED_SOURCES})
target_include_directories(shared PUBLIC .)
source_group("\\" FILES ${SHARED_SOURCES})
