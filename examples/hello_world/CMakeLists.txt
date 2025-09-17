set(TARGET hello-world)

add_executable(${TARGET} hello_world.cpp)
target_link_libraries(${TARGET} PUBLIC lua_thread::lua_thread)

set(LUA_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/hello_world.lua)
get_property(OUTPUT_DIR
    TARGET ${TARGET}
    PROPERTY RUNTIME_OUTPUT_DIRECTORY)
target_post_build_copy(${TARGET} ${LUA_SCRIPT} ${OUTPUT_DIR})