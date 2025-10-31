cmake_minimum_required(VERSION 3.20)
project(banker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()

file(GLOB_RECURSE headers "include/*.hpp")
file(GLOB_RECURSE source "src/*.cpp")

add_executable(banker ${source} ${headers})

target_include_directories(banker PRIVATE "${CMAKE_SOURCE_DIR}/include")

find_package(Threads REQUIRED)
target_link_libraries(banker PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(banker PRIVATE ws2_32)
else()
    target_link_libraries(banker PRIVATE pthread)
endif()