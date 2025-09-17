include(cmake/commands.cmake)

cmake_minimum_required(VERSION 3.20)
project(lua_thread VERSION 0.0.2 LANGUAGES CXX)

# build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
elseif(NOT CMAKE_BUILD_TYPE MATCHES "Debug|Release|RelWithDebInfo|MinSizeRel")
    message(FATAL_ERROR "error: \"${CMAKE_BUILD_TYPE}\" is not a supported value for CMAKE_BUILD_TYPE.")
endif()

find_package(Lua REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET              lua_thread)
set(SOURCE_DIRECTORY    ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIRECTORY   ${CMAKE_SOURCE_DIR}/include)
set(PCH                 ${SOURCE_DIRECTORY}/pch.hpp)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused)

# configuration
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused)

if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
    add_compile_options(-g)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo|MinSizeRel")
    add_compile_options(NDEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_options(-O2)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    add_compile_options(-Oz)
endif()

add_library(${TARGET} STATIC
    ${SOURCE_DIRECTORY}/lua_thread.cpp
)
target_precompile_headers(${TARGET} PUBLIC ${PCH})
target_include_directories(${TARGET} PUBLIC ${INCLUDE_DIRECTORY} ${LUA_INCLUDE_DIR})
target_link_libraries(${TARGET} ${LUA_LIBRARIES})

add_library(${PROJECT_NAME}::${TARGET} ALIAS ${TARGET})

if (LUA_THREAD_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()