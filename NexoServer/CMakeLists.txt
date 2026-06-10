file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp")

set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "src")

add_executable(NexoServer ${SERVER_SOURCES})

target_include_directories(NexoServer PRIVATE
    src
    src/headers
    src/logger
    src/server
    src/session
)

target_link_libraries(NexoServer PRIVATE
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(WIN32)
    target_link_libraries(NexoServer PRIVATE ws2_32 mswsock)
endif()

add_custom_command(TARGET NexoServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config/server.toml"
    "$<TARGET_FILE_DIR:NexoServer>/server.toml"
)