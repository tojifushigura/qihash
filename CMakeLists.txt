cmake_minimum_required(VERSION 3.15)

project(qihash
    VERSION 0.1.0
    DESCRIPTION "Open-source memory-hard hashing and KDF framework"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(QIHASH_BUILD_CLI "Build qihash CLI" ON)
option(QIHASH_BUILD_TESTS "Build qihash tests" ON)
option(QIHASH_BUILD_EXAMPLES "Build qihash examples" ON)
option(QIHASH_ENABLE_WARNINGS "Enable compiler warnings" ON)

add_library(qihash_static STATIC src/qihash.c)
target_include_directories(qihash_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(qihash_static PRIVATE _POSIX_C_SOURCE=200809L)
set_target_properties(qihash_static PROPERTIES OUTPUT_NAME qihash)

if(QIHASH_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(qihash_static PRIVATE /W4)
    else()
        target_compile_options(qihash_static PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

if(QIHASH_BUILD_CLI)
    add_executable(qihash cli/qihash.c)
    target_link_libraries(qihash PRIVATE qihash_static)
    target_compile_definitions(qihash PRIVATE _POSIX_C_SOURCE=200809L)
    if(QIHASH_ENABLE_WARNINGS AND NOT MSVC)
        target_compile_options(qihash PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

if(QIHASH_BUILD_TESTS)
    enable_testing()

    add_executable(test_qihash tests/test_qihash.c)
    target_link_libraries(test_qihash PRIVATE qihash_static)
    add_test(NAME test_qihash COMMAND test_qihash)

    add_executable(test_parser_negative tests/test_parser_negative.c)
    target_link_libraries(test_parser_negative PRIVATE qihash_static)
    add_test(NAME test_parser_negative COMMAND test_parser_negative)

    if(QIHASH_BUILD_CLI)
        add_test(NAME qihash_selftest COMMAND qihash selftest)
    endif()
endif()

if(QIHASH_BUILD_EXAMPLES)
    add_executable(c_example examples/c_example.c)
    target_link_libraries(c_example PRIVATE qihash_static)
endif()

install(TARGETS qihash_static ARCHIVE DESTINATION lib)
install(FILES include/qihash.h DESTINATION include)
if(QIHASH_BUILD_CLI)
    install(TARGETS qihash RUNTIME DESTINATION bin)
endif()
