#include <lua_thread/lua_thread.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
    std::string script = "function Foo(a, b)\nlocal y = a + b\nreturn y + 1\nend";
	lua::lua_thread thrd = lua::lua_thread::from_string(lua::launch::run, script);
    
	// Get global function
	auto foo = thrd.get_global_function<int, int, int>("Foo");
    
	// Call function
    int result = foo(1, 2);
	std::cout << std::format("result = {}\n", result);
    assert(result == 4);    
    
	thrd.join();

	// Undefined behavior if the function is called it this point
	// Function object will be destroyed at this point
}