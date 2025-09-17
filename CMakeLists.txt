include(cmake/commands.cmake)

cmake_minimum_required(VERSION 3.20)
project(lua_thread VERSION 0.0.2 LANGUAGES CXX)

set(TARGET      lua_thread)
set(BINDIR      ${CMAKE_SOURCE_DIR}/bin)
set(SRCDIR      ${CMAKE_SOURCE_DIR}/src)
set(INCLUDEDIR  ${CMAKE_SOURCE_DIR}/include)
set(PCH_FILE    ${SRCDIR}/pch.hpp)
set(SOURCES     ${SRCDIR}/lua_thread.cpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINDIR})

find_package(Lua REQUIRED)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused -g)

add_library(${TARGET} STATIC ${SOURCES})
add_library(${PROJECT_NAME}::${TARGET} ALIAS ${TARGET})
target_precompile_headers(${TARGET} PUBLIC ${PCH_FILE})
target_link_libraries(${TARGET} ${LUA_LIBRARIES} stdc++exp)
target_include_directories(${TARGET} PUBLIC ${INCLUDEDIR} ${LUA_INCLUDE_DIR})

add_subdirectory(examples)