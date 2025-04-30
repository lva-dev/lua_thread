#include "pch.hpp"
#include <lua_thread/lua_thread.hpp>

namespace {

std::string readfile(const std::filesystem::path& path) {
    std::ifstream ifs(path);
    std::stringstream buf; 
    buf << ifs.rdbuf();
    return buf.str();
}

std::string uintptr_to_hex(uintptr_t n, bool prefix = false) {
    std::stringstream ss;
    if (prefix) {
        ss << "0x";
    }
    ss << std::hex << n;
    return ss.str();
}

std::string format_lua_error(const std::string& message) {
    return  "\033[31mlua error\033[0m: " + message + "\n";
}

std::string get_lua_error_from_stack(lua_State* handle) {
    std::string lua_error_message = lua_tostring(handle, -1);
    return "\033[31mlua\033[0m: " + lua_error_message + "\n";
}

void pcall(lua_State* handle, int nargs, int nresults, int msgh) {
    int err = lua_pcall(handle, nargs, nresults, msgh);
    if (err != LUA_OK) {
        return;
    }

    std::string errmsg = format_lua_error("couldn't call procedure");
    errmsg += get_lua_error_from_stack(handle);
    lua_pop(handle, 1);
    switch (err) {    
    case LUA_ERRRUN:
        throw std::runtime_error(errmsg);
    case LUA_ERRMEM:
        throw lua::lua_thread::bad_alloc_error(errmsg);
    case LUA_ERRERR:
        throw std::runtime_error(errmsg);
    }
}

void loadfile(lua_State* handle, const std::filesystem::path& file) {
    std::string s_file = file.string();
    int err = luaL_loadfile(handle, s_file.c_str());

    if (err == LUA_OK) {
        return;        
    }

    std::string errmsg = format_lua_error("could not load file from \'" + file.string() + '\'');
    switch (err) {
    case LUA_ERRFILE:
        throw std::runtime_error(errmsg);
    case LUA_ERRSYNTAX:
        throw lua::lua_thread::lua_syntax_error(errmsg);
    case LUA_ERRMEM:
        throw lua::lua_thread::bad_alloc_error(errmsg);
    }       
}

void loadstring(lua_State* handle, const std::string& str) {
    int err = luaL_loadstring(handle, str.c_str());
    if (err == LUA_OK) {
        return;        
    }

    std::string errmsg = format_lua_error("could not load string from \'" + str + '\'');
    switch (err) {
    case LUA_ERRSYNTAX:
        throw lua::lua_thread::lua_syntax_error(errmsg);
    case LUA_ERRMEM:
        throw lua::lua_thread::bad_alloc_error(errmsg);
    }       
}

void dofile(lua_State* handle, const std::filesystem::path& file) {
    loadfile(handle, file);
    pcall(handle, 0, LUA_MULTRET, 0);
}

void dostring(lua_State* handle, const std::string& str) {
    loadstring(handle, str);
    pcall(handle, 0, LUA_MULTRET, 0);
}

void close(lua_State* handle) {
    lua_close(handle);
}

};

namespace lua {

constexpr bool nil::operator==(nil) const { 
    return true; 
}

constexpr bool nil::operator!=(nil) const { 
    return false;
}

lua_thread::lua_syntax_error::lua_syntax_error(const std::string& what_arg) : std::runtime_error(what_arg)
{}

//////////////////////// lua_thread::bad_load_error ////////////////////////

lua_thread::bad_alloc_error::bad_alloc_error(const std::string& what_arg) : std::runtime_error(what_arg) 
{}

//////////////////////// lua_thread::id ////////////////////////

lua_thread::id::id()  
{}

lua_thread::id::id(const lua_State* ptr_to_internal) 
: m_id(reinterpret_cast<uintptr_t>(ptr_to_internal))
{}

//////////////////////// lua_thread ////////////////////////

lua_thread::lua_thread()
{}


lua_thread::lua_thread(lua_thread&& other) {
    m_state = std::exchange(other.m_state, nullptr);
    m_id = std::exchange(other.m_id, id());
    return;
}

lua_State* create_handle() {
    lua_State* new_handle = luaL_newstate();
    if (new_handle == nullptr) {
        throw std::bad_alloc();
    }

    luaL_openlibs(new_handle);
    return new_handle;
}

lua_thread lua_thread::from_path(const std::filesystem::path& file) {
    lua_thread new_thread;
    dofile(new_thread.internal_state(), file);
    return new_thread;
}

lua_thread lua_thread::from_string(const std::string& source) {
    lua_thread new_thread;
    dostring(new_thread.internal_state(), source);
    return new_thread;
}

lua_thread& lua_thread::operator=(lua_thread&& other) {
    if (joinable()) {
        std::terminate();
    }

    m_state = std::exchange(other.m_state, nullptr);
    m_id = std::exchange(m_id, id());

    return *this;
}

lua_thread::~lua_thread() {
    close(m_state);
    if (joinable()) {
        std::terminate();
    }
}

void lua_thread::start() {
    if (joinable()) {
        return;
    }

    m_state = create_handle();
}

bool lua_thread::joinable() const {
    return get_id() == id();
}

void lua_thread::join() {
    close(internal_state());
    m_id = id();
}

void lua_thread::detatch() {
    m_id = id();
}

lua_thread::id lua_thread::get_id() const {
    return m_id;
}

lua_State* lua_thread::internal_state() const {
    return m_state;
}

void lua_thread::swap(lua_thread& other) {
    std::swap(this->m_state, other.m_state);
    std::swap(this->m_id.m_id, other.m_id.m_id);
}

template<LuaType R, LuaType...  Args>
R lua_thread::call_lua_function(const std::string& name, const Args&... args) {
    // get function
    int err0 = lua_getglobal(m_state, name.c_str());
    if (err0) {
        throw; // (get_lua_error_message(""));
    }

    // push each argument onto stack
    // by folding args from the right
    (m_push_type(args), ...);
    
    constexpr std::size_t n_args = sizeof...(args);
    constexpr std::size_t n_ret = 1;
    int err1 = lua_pcall(internal_state(), n_args, n_ret, 0);
    if (err1 != 0) {
        throw; // (get_lua_error_message("failed function \'" + name + "\' failed"));
    }
        
    // get result and pop
    R result = m_to_type<R>(-1);
    lua_pop(internal_state(), 1);
    return result;
}

template<LuaType T, LuaType Expected>
void lua_thread::m_is_type(T value) const {
    static_assert(LuaType<Expected>, "T is not a valid lua type");    
    return std::is_same_v<T, Expected>;
}

template<LuaType T>
void lua_thread::m_push_type(T value) {
    if constexpr (NilType<T>) {
        lua_pushnil(internal_state(), value);
    } else if constexpr (BooleanType<T>) {
        lua_pushboolean(internal_state(), value);
    } else if constexpr (NumberType<T>) {
        lua_pushnumber(internal_state(), value);
    } else if constexpr (StringType<T>) {
        lua_pushstring(internal_state(), value);
    } else {
        static_assert(false, "T is not a valid lua type");
    }
}

template<LuaType To>
To lua_thread::m_to_type(int pos) {
    if constexpr (NilType<To>) {
        return NIL;
    } else if constexpr (BooleanType<To>) {
        return lua_toboolean(internal_state(), pos);
    } else if constexpr (NumberType<To>) {
        int isnum;
        To x = lua_tonumberx(internal_state(), pos, &isnum);
    } else if constexpr (StringType<To>) {
        return lua_tostring(internal_state(), pos);
    } else {
        static_assert(false, "T is not a valid lua type");
    }
}

};