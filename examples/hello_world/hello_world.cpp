#include <lua_thread/lua_thread.hpp>

#include <filesystem>


int main() {
	const auto directory = std::filesystem::read_symlink("/proc/self/exe").parent_path();

	// Creating an inactive thread; can be assigned later.
	lua::lua_thread thread_1 {};
    
	// Creating a thread from script (loaded from file)
    static const std::string SCRIPT_NAME = "hello_world.lua";
	lua::lua_thread thread_2 = lua::lua_thread::from_file(lua::launch::run, directory / SCRIPT_NAME);
    
	// thread_1.join(); // this will throw a std::system_error since thread1 is not running
	thread_2.join();
}