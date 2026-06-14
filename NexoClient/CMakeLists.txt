file(GLOB_RECURSE CLIENT_SOURCES "src/*.cpp")

set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "src")

add_executable(NexoClient ${CLIENT_SOURCES})

target_include_directories(NexoClient PRIVATE
    src
    src/headers
    src/logger
    src/utils
    src/servers/socks5server
    src/sessions/socks5session
)

target_link_libraries(NexoClient PRIVATE
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(WIN32)
    target_link_libraries(NexoClient PRIVATE ws2_32 mswsock)
endif()

add_custom_command(TARGET NexoClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config/client.toml"
    "$<TARGET_FILE_DIR:NexoClient>/client.toml"
)