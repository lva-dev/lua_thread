if (LUA_THREAD_EXAMPLES_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LUA_THREAD_EXAMPLES_OUTPUT_DIRECTORY})
endif()

add_subdirectory(hello_world)
add_subdirectory(global_functions)