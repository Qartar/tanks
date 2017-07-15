add_library(physics STATIC
    p_collide.cpp
    p_collide.h
    p_material.h
    p_rigidbody.cpp
    p_rigidbody.h
    p_shape.h
    p_trace.cpp
    p_trace.h
)

target_link_libraries(physics PUBLIC shared)
target_include_directories(physics PUBLIC .)
