file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_executable(nexo_client ${CLIENT_SOURCES})

set_target_properties(nexo_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_SOURCE_DIR}/bin/debug"

    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_SOURCE_DIR}/bin/release"
)

target_include_directories(nexo_client PRIVATE
    src
    src/headers
    src/logger
    src/utils
    src/servers/socks5server
    src/sessions/socks5session
)

target_link_libraries(nexo_client PRIVATE
    Boost::headers
    OpenSSL::SSL
    OpenSSL::Crypto
    tomlplusplus::tomlplusplus
)

if(MSVC)
    target_compile_options(nexo_client PRIVATE
        /EHsc
    )
endif()

if(WIN32)
    target_link_libraries(nexo_client PRIVATE
        ws2_32
        mswsock
    )
endif()

add_custom_command(TARGET nexo_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config/client.toml"
    "$<TARGET_FILE_DIR:nexo_client>/config/client_configuration.toml"
)