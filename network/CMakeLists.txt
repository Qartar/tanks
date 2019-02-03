set(NETWORK_SOURCES
    net_address.cpp
    net_address.h
    net_channel.cpp
    net_channel.h
    net_message.cpp
    net_message.h
    net_socket.cpp
    net_socket.h
)

add_library(network STATIC ${NETWORK_SOURCES})
target_link_libraries(network PUBLIC shared)
target_include_directories(network PUBLIC .)
source_group("\\" FILES ${NETWORK_SOURCES})
