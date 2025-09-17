#pragma once

#include <lua.h>

#include <compare>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

// Helper Macros
#define never_reach(...) static_assert(false __VA_OPT__(,) __VA_ARGS__)

// Forward declaration
struct lua_State;

namespace lua {
	//////////////////////// Forward Declarations ////////////////////////

	class lua_type_info;
	class lua_thread;
	template<class> struct global_function;

	namespace _detail { inline lua_type_info* new_lua_type_info(int type_id); }

	//////////////////////// Types ////////////////////////

	using boolean_t = bool;
	using integer_t = int64_t;
	using number_t = double;
	using string_t = std::string;

	struct nil_t {
		inline constexpr bool operator==(nil_t) const { return true; }

		inline constexpr bool operator!=(nil_t) const { return false; }

		template<class CharT, class Traits>
		friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, nil_t nil);
	};

	// lua nil constant
	constexpr nil_t nil;

	//////////////////////// Type Traits and Concepts ////////////////////////

	/// Type Traits

	template<class T> struct is_nil : std::bool_constant<std::same_as<T, nil_t>> {};

	template<class T> constexpr bool is_nil_v = is_nil<T>::value;

	template<class T> struct is_boolean : std::bool_constant<std::same_as<T, boolean_t>> {};

	template<class T> constexpr bool is_boolean_v = is_boolean<T>::value;

	template<class T> struct is_integer : std::bool_constant<std::convertible_to<T, integer_t> && std::integral<T>> {};

	template<class T> constexpr bool is_integer_v = is_integer<T>::value;

	template<class T>
	struct is_number : std::bool_constant<std::convertible_to<T, number_t> && std::floating_point<T>> {};

	template<class T> constexpr bool is_number_v = is_number<T>::value;

	template<class T>
	struct is_string
		: std::bool_constant<
			  std::is_constructible_v<string_t, T> &&
			  (std::is_same_v<T, std::string> || std::is_same_v<T, char*> || std::is_same_v<T, const char*>)> {};

	template<class T> constexpr bool is_string_v = is_string<T>::value;

	template<class T>
	struct is_lua
		: std::bool_constant<is_nil_v<T> || is_boolean_v<T> || is_integer_v<T> || is_number_v<T> || is_string_v<T>> {};

	template<class T> constexpr bool is_lua_v = is_lua<T>::value;

	/// Concepts

	template<class T>
	concept nil_like = is_nil_v<T>;

	template<class T>
	concept boolean_like = is_boolean_v<T>;

	template<class T>
	concept integer_like = is_integer_v<T>;

	template<class T>
	concept number_like = is_number_v<T>;

	template<class T>
	concept string_like = is_string_v<T>;

	template<class T>
	concept lua_like = is_lua_v<T>;

	//////////////////////// Type Conversion Functions ////////////////////////

	inline constexpr nil_t to_nil(const auto&);

	template<string_like R = string_t, class T> inline constexpr R to_string(const T& value);

	template<number_like R = number_t, class T> inline R to_number(const T& value);

	template<class R = boolean_t, class T> inline constexpr R to_boolean(const T& value);

	//////////////////////// Exceptions ////////////////////////

	class lua_syntax_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	class memory_alloc_error : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class thread_yield : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class message_handler_error : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class file_error : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	template<lua_like R, lua_like... Args> struct global_function<R(Args...)> {
	public:
		using result_type = R;
        static constexpr int nargs = sizeof...(Args);
        static constexpr int nresults = 1;
        
		global_function(lua_thread& environment, const std::string& fname);
		~global_function();

		R operator()(Args... args) const;
	private:
		lua_thread& m_thread;
		int m_registry_key;
	};

	enum class launch {
		run = 0x1,
		load = 0x2
	};

	// enum class libs {
	// 	basic = 0x001,
	// 	coroutine = 0x002,
	// 	package = 0x004,
	// 	string = 0x008,
	// 	utf8 = 0x010,
	// 	table = 0x020,
	// 	math = 0x040,
	// 	io = 0x080,
	// 	os = 0x100,
	// 	debug = 0x200,
	//     all = basic | coroutine | package | string | utf8 | table | math | io | os | debug
	// };

	/**
	 */
	class lua_type_info {
	public:
		lua_type_info() = delete;

		const char* name() const;
		int dynamic_type() const;
	private:
		int m_lua_type_id;
		lua_State* m_state;
		const char* m_type_name;

		friend inline lua_type_info* _detail::new_lua_type_info(int lua_type_id);

		lua_type_info(lua_State* state, int lua_type_id);
	};

	template<lua_like T> const lua_type_info& lua_typeid();
	template<lua_like T> const lua_type_info& lua_typeid(const T& expression);

	//////////////////////// lua_thread ////////////////////////
	class lua_thread {
	public:
		class id;

		// Constructors and Destructor
		lua_thread() = default;
		lua_thread(const lua_thread& other) = delete;
		lua_thread& operator=(const lua_thread& other) = delete;
		lua_thread(lua_thread&& other);
		lua_thread& operator=(lua_thread&& other);
		~lua_thread();
		// static lua_thread from_file(const std::filesystem::path& source_path);
		// static lua_thread from_string(const std::string& source);
		static lua_thread from_file(launch policy, const std::filesystem::path& source_path);
		static lua_thread from_string(launch policy, const std::string& source);

		// Observers
		bool joinable() const noexcept;
		id get_id() const noexcept;
		lua_State* internal_state() const noexcept;

		// Operations
		void join();
		void detatch() noexcept;
		void swap(lua_thread& other);

		template<lua_like T> T get_global(const std::string& name);
		template<lua_like R, lua_like... Args> global_function<R(Args...)> get_global_function(const std::string& name);
	private:
		lua_State* m_state;
		launch m_launch_policy;
		std::thread m_running_thread;

		static lua_thread new_initialized(launch launch_policy);
		void load_string_as_chunk(const std::string& source);
		void load_file_as_chunk(const std::filesystem::path& file);
		void pcall_chunk();
		[[nodiscard]] std::thread pcall_chunk_async();
		void await_pcall();

	}; // class lua_thread

	inline void swap(lua_thread& a, lua_thread& b) { a.swap(b); }

	class lua_thread::id {
	public:
		id() noexcept = default;
		~id() noexcept = default;
		id(const id&) noexcept = default;
		id& operator=(const id&) noexcept = default;
		id(id&&) noexcept;
		id& operator=(id&&) noexcept;

		bool operator==(id other) const noexcept;
		std::strong_ordering operator<=>(id other) const noexcept;

		template<class CharT, class Traits>
		friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const id& nil);
	private:
		std::size_t m_id;

		friend lua_thread;

		id(const lua_thread& thread);
	}; // class id

	namespace _detail {
		// A single Lua state that can be used for accessing global operations like `lua_typename`
		extern std::unique_ptr<lua_State, void (*)(lua_State*)> global_lua_state;

		// A group of static objects which hold RTTI for each Lua type
		inline std::unique_ptr<lua_type_info> lti_NIL {new_lua_type_info(LUA_TNIL)};
		inline std::unique_ptr<lua_type_info> lti_NUMBER {new_lua_type_info(LUA_TNUMBER)};
		inline std::unique_ptr<lua_type_info> lti_BOOLEAN {new_lua_type_info(LUA_TBOOLEAN)};
		inline std::unique_ptr<lua_type_info> lti_STRING {new_lua_type_info(LUA_TSTRING)};
		inline std::unique_ptr<lua_type_info> lti_TABLE {new_lua_type_info(LUA_TTABLE)};
		inline std::unique_ptr<lua_type_info> lti_FUNCTION {new_lua_type_info(LUA_TFUNCTION)};
		inline std::unique_ptr<lua_type_info> lti_USERDATA {new_lua_type_info(LUA_TUSERDATA)};
		inline std::unique_ptr<lua_type_info> lti_THREAD {new_lua_type_info(LUA_TTHREAD)};
		inline std::unique_ptr<lua_type_info> lti_LIGHTUSERDATA {new_lua_type_info(LUA_TLIGHTUSERDATA)};

		/**
		 * Returns a const reference to a static-storage `lua_type_info` object that corresponds with the given `type`.
		 */
		inline constexpr const lua_type_info& get_type_info(int type);

		/**
		 * Throws an `std::exception` that corresponds with `lua_error_type`.
		 *
		 * The behavior is undefined if `lua_error_type` is not a valid value.
		 * @param lua_error_type the error type. It must be one of the following values:
		 *   - `LUA_YIELD` (for `lua::thread_yield`)
		 *   - `LUA_ERRRUN` (for `std::runtime_error`)
		 *   - `LUA_ERRSYNTAX` (for `lua::lua_syntax_error`)
		 *   - `LUA_ERRMEM` (for `lua::memory_alloc_error`)
		 *   - `LUA_ERRERR` (for `lua::message_handler_error`)
		 *   - `LUA_ERRFILE` (for `lua::file_error`)
		 * @param message the message associated with the exception.
		 */
		inline void throw_exception(int lua_error_type, const std::string& message = "");

		/**
		 * Contains an integer that represents the Lua C API's equivalent of `T`
		 * @return an integer that best represents `T` in the Lua C API
		 */
		template<lua_like T> struct to_dynamic_type {
			static constexpr int value = [] {
				if constexpr (nil_like<T>) {
					return LUA_TNIL;
				} else if constexpr (boolean_like<T>) {
					return LUA_TBOOLEAN;
				} else if constexpr (number_like<T> || integer_like<T>) {
					return LUA_TNUMBER;
				} else if constexpr (string_like<T>) {
					return LUA_TSTRING;
				} else {
					never_reach("lua_thread: invalid lua type");
				}
			}();
		};

		template<lua_like T> constexpr int to_lua_type_v = to_dynamic_type<T>::value;

		namespace stack {
			/**
			 * Push a value to the top of a Lua stack.
			 */
			template<lua_like T> inline void push(lua_State* state, const T& value);

			/**
			 * Copy a value from a Lua stack and return it.
			 */
			template<lua_like T> inline T to(lua_State* state, int pos);

            /**
             * Pushes the value at the specified registry key to the stack.
             * Returns the dynamic (Lua C API) type of the pushed value.
             */
            int index_registry(lua_State* state, int key);
		} // namespace stack

		inline void start_state(lua_State* state);
	} // namespace _detail
}; // namespace lua

#include "lua_thread.tcc"
