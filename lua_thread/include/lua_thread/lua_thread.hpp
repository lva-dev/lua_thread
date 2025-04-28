#include <string>
#include <string_view>
#include <stdexcept>

// Forward Declarations

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
concept is_lua_nil = std::is_same_v<T, lua::nil>;

template<typename T> 
concept is_lua_boolean = std::is_same_v<T, bool>;

template<typename T> 
concept is_lua_number = std::is_arithmetic_v<T>;

template<typename T>
concept is_lua_string = 
    std::is_same_v<T, std::basic_string<typename T::value_type>>
    || std::is_same_v<T, const char*>;

template<typename T>
concept is_a_lua_type = 
    is_lua_nil<T> 
    || is_lua_boolean<T> 
    || is_lua_number<T> 
    || is_lua_string<T>;

    
template<typename... T>
concept is_multiple_lua = requires(T... args) { (is_a_lua_type<T> || ...) && (sizeof...(args) > 1); };

class lua_syntax_error : public std::domain_error {
    lua_syntax_error(const std::string& what_arg);
};

class bad_load_error : public std::runtime_error {
    bad_load_error(const std::string& what_arg);
}; // // class lua::thread::bad_load_error

class lost_thread_error : public std::runtime_error {
    lost_thread_error(const std::string& what_arg);
}; // class lua::thread::lost_thread_error

//////////////////////// lua_thread ////////////////////////

class lua_thread {
public:
    class id {
    public:
        id();
        auto operator<=>(const id& other) const = default;
    private:
        friend class lua_thread;
        inline static constexpr uintptr_t DEFAULT_ID = 0;
        id(const lua_State* internal_lua_state);
        id& operator=(id&& other);
        uintptr_t m_id;
    }; // class lua::thread::id
    
    lua_thread();
    lua_thread(const std::filesystem::path& source_path); 
    lua_thread(std::string_view source) ;
    lua_thread(const lua_thread&& other);
    lua_thread& operator=(const lua_thread&& other);
    ~lua_thread();
    bool joinable() const;
    void join();
    void detatch();
    id get_id() const;
    lua_State* get_state(); 

    template<is_a_lua_type R, is_a_lua_type...  Args>
    R call_lua_function(std::string_view name, Args... args);
private:
    lua_State* m_state;
    bool m_joinable;
    id m_id;
    
    lua_State static lua_thread::m_create_state();

    std::string get_lua_error_message(const std::string& message);

    template<typename T, typename Expected>
    void m_is_type(T value) const;

    template<typename T>
    void m_push_type(T value);

    template<typename T>
    T m_to_type(int pos);

}; // class lua::thread

}; // namespace lua