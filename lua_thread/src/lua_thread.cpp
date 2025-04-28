#include "PCH.hpp"
#include <lua_thread/lua_thread.hpp>

namespace {

std::string read_whole_file(const std::filesystem::path& path) {
    std::ifstream ifs(path);
    std::stringstream buf; 
    buf << ifs.rdbuf();
    return buf.str();
}

std::string int_to_hex(size_t n) {
    std::stringstream ss;
    ss << std::hex << n;
    return ss.str();
}

};

namespace lua {

constexpr bool nil::operator==(nil) const { 
    return true; 
}

constexpr bool nil::operator!=(nil) const { 
    return false;
}

lua_syntax_error::lua_syntax_error(const std::string& what_arg) : std::domain_error(what_arg)
{}

//////////////////////// lua_thread::bad_load_error ////////////////////////

bad_load_error::bad_load_error(const std::string& what_arg) : std::runtime_error(what_arg) 
{}

//////////////////////// lua_thread::lost_thread_error ////////////////////////

lost_thread_error::lost_thread_error(const std::string& what_arg) : std::runtime_error(what_arg) 
{}

//////////////////////// lua_thread::id ////////////////////////

lua_thread::id::id()
:  
{}

lua_thread::id::id(const lua_State* ptr_to_internal) 
: m_id(reinterpret_cast<uintptr_t>(ptr_to_internal))
{}

//////////////////////// lua_thread ////////////////////////

lua_thread::lua_thread() : m_joinable(false)
{}

lua_thread::lua_thread(const std::filesystem::path& source_path) 
: m_state(m_create_state()),
  m_joinable(true),
  m_id(m_state) {
    // read script
    std::string src = read_whole_file(source_path);
    
    // load script
    const char* buff = src.c_str();
    std::size_t buff_size = src.size();
    const char* buff_name = source_path.filename().string().c_str();
    int errc = luaL_loadbuffer(m_state, buff, buff_size, buff_name);
    if (errc != 0) {
        throw std::bad_alloc(get_lua_error_message("could not load \'" + source_path.string() + "\'"));
    }       
}

lua_thread::lua_thread(std::string_view source) 
: m_state(m_create_state()),
  m_joinable(true),
  m_id(m_state) {
    // read script
    std::string src = read_whole_file(source);
    
    // load script
    const char* buff = src.c_str();
    std::size_t buff_size = src.size();
    std::string buff_name = "0x" + int_to_hex(m_id);
    int errc = luaL_loadbuffer(m_state, buff, buff_size, buff_name.c_str());
    if (errc != 0) {
        throw bad_load_error(get_lua_error_message("could not load script"));
    }        
}

lua_thread::lua_thread(const lua_thread&& other) {
    *this = other;
    return 
}

lua_thread& lua_thread::operator=(const lua_thread&& other) {
    m_state = std::exchange(other.m_state, nullptr);
    m_joinable = std::exchange(other.m_joinable, false);
    m_id = std::move();

    m_state = other.m_state;
    m_joinable = other.m_joinable;
    m_id = other.m_id;
    
    other.m_state = nullptr;
    other.m_joinable = false;
    other.m_id.m_id = id();
    
    return;
}

lua_thread::~lua_thread() {
    lua_close(m_state);
    if (m_joinable) {
        std::terminate();
    }
}

bool lua_thread::joinable() const {
    return m_joinable;
}

void lua_thread::join() {
    lua_close(m_state);
    m_joinable = false;
}

void lua_thread::detatch() {
    m_joinable = false;
}

lua_thread::id lua_thread::get_id() const {
    return m_id;
}

lua_State* lua_thread::get_state() {
    return m_state;
}

template<typename R, typename...  Args>
R lua_thread::call_lua_function(std::string_view name, Args... args) {
    // get function
    int err = lua_getglobal(m_state, name.c_str());
    if (err) {
        throw (get_lua_error_message(""));
    }

    // push each argument onto stack
    // by folding args from the right
    (m_push_type(args), ...)

    // call function
    lua_call(m_state, , 1);
    
    constexpr std::size_t n_args = sizeof...(args);
    constexpr std::size_t n_ret = 1;
    int err = lua_pcall(lua_state.L, n_args, n_ret, 0)
    if (err != 0) {
        throw (get_lua_error_message("failed function \'" + name + "\' failed"));
    }
        
    // get result and pop
    R result = m_to_type(-1);
    lua_pop(m_state, 1);
    return result;
}

lua_State* lua_thread::m_create_state() {
    lua_State* new_lua_state = luaL_newstate();
    if (new_lua_state == nullptr) {
        return;
    }

    luaL_openlibs(new_lua_state);
    return new_lua_state;    
}

std::string lua_thread::get_lua_error_message(const std::string& message) {
    const char* lua_error_message = lua_tostring(m_state, -1);
    lua_pop(m_state, 1);
    return  "\033[31merror\033[0m: " + message + "\n"
        "\033[31mlua\033[0m: " + lua_error_message + ")\n";
}

template<typename T, typename Expected>
void lua_thread::m_is_type(T value) const {
    if constexpr (!is_a_lua_type<Expected>) {
        static_assert(false, "T is not a valid lua type");
    }
    
    return std::is_same_v<T, Expected>;
}

template<typename T>
void lua_thread::m_push_type(T value) {
    if constexpr (is_lua_nil<T>) {
        lua_pushnil(m_state, value);
    }
    else if constexpr (is_lua_boolean<T>) {
        lua_pushboolean(m_state, value);
    }
    else if constexpr (is_lua_number<T>) {
        lua_pushnumber(m_state, value);
    }
    else if constexpr (is_lua_string<T>) {
        lua_pushstring(m_state, value);
    } else constexpr {
        static_assert(false, "T is not a valid lua type");
    }
}

template<typename To>
To lua_thread::m_to_type(int pos) {
    if constexpr (is_lua_nil<To>) {
        return nil;
    }
    else if constexpr (is_lua_boolean<To>) {
        return lua_toboolean(m_state, value);
    }
    else if constexpr (is_lua_number<To>) {
        return lua_tonumber(m_state, value);
    }
    else if constexpr (is_lua_string<To>) {
        return lua_tostring(m_state, value);
    } else constexpr {
        static_assert(false, "T is not a valid lua type");
    }
}

};
};