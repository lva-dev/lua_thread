# lua_thread
An C++ object-oriented and type-safe interface to Lua's C API, modeled after std::thread.
## Building and Linking
Clone lua_thread into your C++ project as a submodule:
```
git clone --recurse-submodules https://github.com/lva-dev/lua_thread.git vendor/lua_thread
```
then add it to your project's `CMakeLists.txt`:
```cmake
project(PROJECT_NAME)

# add lua_thread as a subdirectory
add_subdirectory(vendor/lua_thread)

set(TARGET target)

add_executable(${TARGET} src/main.cpp)
# link lua_thread
target_link_libraries(${TARGET} PUBLIC lua_thread::lua_thread)
```
Then build your CMake project as normal.

See [build-cmake.md](docs/build-cmake.md) for lua_thread-specific CMake options.
## License
lua_thread is distributed under the terms of the MIT License.
