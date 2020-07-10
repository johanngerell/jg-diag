cmake_minimum_required(VERSION 3.10)

project(jg_diag)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
endif()

include_directories(src ../jg/inc)

if(MSVC)
    string(REGEX REPLACE "/W[1-3]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

add_definitions(-DJG_VERIFY_ENABLE_STACK_TRACE -DJG_VERIFY_ENABLE_TERMINATE)

if (MSVC)
    add_compile_options(/Zc:__cplusplus /EHsc /W4 /WX)
else()
    # jg::mock can't be used with -pedantic due to "the ##__VA_ARGS__ trick"
    add_compile_options(-Wall -Wextra -Werror)
endif()

add_executable(jg_diag src/main.cpp)
