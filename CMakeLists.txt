cmake_minimum_required(VERSION 3.20)
project(banker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wcast-align -Wnon-virtual-dtor)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffunction-sections -fdata-sections")# -flto
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections -s -flto")
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "include/*.hpp")
file(GLOB_RECURSE VENDOR_SOURCES "src/banker/vendor/**/*.cpp" "src/banker/vendor/**/*.c")
list(REMOVE_ITEM SOURCES ${VENDOR_SOURCES})

set_source_files_properties(${VENDOR_SOURCES} PROPERTIES COMPILE_FLAGS "-w")

find_package(Threads REQUIRED)

function(add_banker_variant TARGET DEFINE)
    add_executable(${TARGET} ${SOURCES} ${VENDOR_SOURCES})
    target_include_directories(${TARGET} PRIVATE include)
    target_compile_definitions(${TARGET} PRIVATE ${DEFINE})
    target_link_libraries(${TARGET} PRIVATE Threads::Threads)
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE ws2_32 bcrypt)
    endif()
endfunction()

add_banker_variant(banker_client    BUILD_CLIENT)
add_banker_variant(banker_server    BUILD_SERVER)
add_banker_variant(banker_tests     BUILD_TESTS)
add_banker_variant(banker_http      BUILD_HTTP_FILE_SERVER)