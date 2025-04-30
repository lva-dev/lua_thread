#pragma once

#include <string>
#include <stdexcept>
#include <filesystem>

// Forward declaration
struct lua_State;

// lua 
namespace lua {

//////////////////////// Types ////////////////////////

struct nil {
    constexpr bool operator==(nil) const;
    constexpr bool operator!=(nil) const;
};

constexpr nil NIL;
    
using boolean = bool;
using number = double;
using string = std::string;

//////////////////////// Concepts ////////////////////////

template<typename T> 
concept NilType = std::same_as<T, lua::nil>;

template<typename T> 
concept BooleanType = std::same_as<T, bool>;

template<typename T> 
concept NumberType = std::is_convertible_v<T, double> &&
    !std::is_same_v<T, bool> &&
    !std::is_same_v<T, char> &&
    !std::is_same_v<T, signed char> &&
    !std::is_same_v<T, unsigned char>;

template<typename T>
concept StringType = 
    std::is_same_v<T, std::basic_string<typename T::value_type>>
    || std::is_same_v<T, const char*>;

template<typename T>
concept LuaType = 
    NilType<T> 
    || BooleanType<T> 
    || NumberType<T> 
    || StringType<T>;
    
template<typename... T>
concept MultipleLuaTypes = requires(T... args) { (LuaType<T> || ...) && (sizeof...(args) > 1); };

//////////////////////// lua_thread ////////////////////////

// For specializing std::swap for lua_thread;
using std::swap;

class lua_thread {
public:
    class lua_syntax_error : std::runtime_error {
    public:
        lua_syntax_error(const std::string& what_arg);
    }; // class lua_syntax_error
    
    class bad_alloc_error : std::runtime_error {
    public:
        bad_alloc_error(const std::string& what_arg);
    }; // class bad_alloc_error
public:
    class id {
    public:
        id();
        auto operator<=>(const id& other) const = default;
    private:
        friend class lua_thread;
        id(const lua_State* internal_lua_state);
        uintptr_t m_id;
    }; // class id
public:
    lua_thread();
    lua_thread(lua_thread&& other);
    lua_thread(const lua_thread& other) = delete;
    static lua_thread from_path(const std::filesystem::path& source_path);
    static lua_thread from_string(const std::string& source);
    lua_thread& operator=(lua_thread&& other);
    ~lua_thread();
    
    void start();
    bool joinable() const;
    id get_id() const;
    lua_State* internal_state() const; 

    void join();
    void detatch();
    void swap(lua_thread& other);

    template<LuaType R, LuaType...  Args>
    R call_lua_function(const std::string& name, const Args&... args);
private:
    lua_State* m_state;
    id m_id;

    template<LuaType T, LuaType Expected>
    void m_is_type(T value) const;

    template<LuaType T>
    void m_push_type(T value);

    template<LuaType To>
    To m_to_type(int pos);

}; // class lua_thread

}; // namespace lua