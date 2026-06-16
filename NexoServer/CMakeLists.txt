file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_executable(nexod ${SERVER_SOURCES})

set_target_properties(nexod PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_SOURCE_DIR}/bin/debug"

    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_SOURCE_DIR}/bin/release"
)

target_include_directories(nexod PRIVATE
    src
    src/headers
    src/logger
    src/server
    src/session
    src/utils
)

target_link_libraries(nexod PRIVATE
    Boost::headers
    OpenSSL::SSL
    OpenSSL::Crypto
    tomlplusplus::tomlplusplus
)

target_compile_options(nexod PRIVATE
    /EHsc
)

if(WIN32)
    target_link_libraries(nexod PRIVATE
        ws2_32
        mswsock
    )
endif()

add_custom_command(TARGET nexod POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config/server.toml"
    "$<TARGET_FILE_DIR:nexod>/config/server_configuration.toml"
)