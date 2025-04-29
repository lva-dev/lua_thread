#include "pch.hpp"

int main() {
    lua::nil nil_value = lua::NIL;
    lua::number n_value = 3.14;
    lua::string s_value = "abcde fghij";
    lua::boolean b_value = false;
    
    lua::lua_thread thread1{};
    thread1.start();

    lua::lua_thread thread2 = lua::lua_thread::from_string("io.write(\"Thread 1 has been created\")");

    lua::lua_thread thread3 = lua::lua_thread::from_path("example.lua");

    thread2.join();
    thread3.join();
}