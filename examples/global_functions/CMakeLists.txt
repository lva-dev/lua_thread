set(TARGET global-functions)

add_executable(${TARGET} global_functions.cpp)
target_link_libraries(${TARGET} PUBLIC lua_thread::lua_thread)