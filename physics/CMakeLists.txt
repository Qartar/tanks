set(PHYSICS_SOURCES
    p_collide.cpp
    p_collide.h
    p_material.h
    p_rigidbody.cpp
    p_rigidbody.h
    p_shape.h
    p_trace.cpp
    p_trace.h
)

add_library(physics STATIC ${PHYSICS_SOURCES})
target_link_libraries(physics PUBLIC shared)
target_include_directories(physics PUBLIC .)
source_group("\\" FILES ${PHYSICS_SOURCES})
