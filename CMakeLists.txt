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

find_package(Threads REQUIRED)

function(add_banker_variant target_name compile_def)
    add_executable(${target_name} ${source} ${headers})
    target_include_directories(${target_name} PRIVATE "${CMAKE_SOURCE_DIR}/include")
    target_compile_definitions(${target_name} PRIVATE ${compile_def})
    target_link_libraries(${target_name} PRIVATE Threads::Threads)

    if(WIN32)
        target_link_libraries(${target_name} PRIVATE ws2_32)
    else()
        target_link_libraries(${target_name} PRIVATE pthread)
    endif()
endfunction()

add_banker_variant(banker_client BUILD_CLIENT)
add_banker_variant(banker_server BUILD_SERVER)