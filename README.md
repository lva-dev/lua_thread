# lua_thread

## Building Independently
Create a `build` directory and run CMake.
```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```
## Linking
Clone lua_thread into your C++ project as a submodule:
```
git clone --recurse-submodules https://github.com/lva-dev/lua_thread.git vendor/lua_thread
```
then add it to your CMakeLists:
```cmake
project(PROJECT_NAME)

# add lua_thread as a subdirectory
add_subdirectory(vendor/lua_thread)

add_executable(PROJECT_TARGET src/main.cpp)
# link lua_thread
target_link_libraries(lua_thread)
```
## License
lua_thread is distributed under the terms of the MIT License.