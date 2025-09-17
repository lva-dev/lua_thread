if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY examples)
else()
    set(examples)
endif()

add_subdirectory(hello_world)
add_subdirectory(global_functions)