#include <lua_thread/lua_thread.hpp>

#include <lua.hpp>

#include <cassert>
#include <concepts>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace lua {
	namespace _detail {
		inline void throw_exception(int lua_error_type, const std::string& message) {
			switch (lua_error_type) {
			case LUA_YIELD:
				throw lua::thread_yield(message);
			case LUA_ERRRUN:
				throw std::runtime_error(message);
			case LUA_ERRSYNTAX:
				throw lua::lua_syntax_error(message);
			case LUA_ERRMEM:
				throw lua::memory_alloc_error(message);
			case LUA_ERRERR:
				throw lua::message_handler_error(message);
			case LUA_ERRFILE:
				throw lua::file_error(message);
			default:
				std::cerr << "lua_thread: reached unreachable control path: invalid lua type";
				std::unreachable();
			}
		}

		inline lua_type_info* new_lua_type_info(int lua_type_id) {
			return new lua_type_info {_detail::global_lua_state.get(), lua_type_id};
		}

		inline constexpr const lua_type_info& get_type_info(int type) {
			switch (type) {
			case LUA_TNIL:
				return *lti_NIL;
			case LUA_TNUMBER:
				return *lti_NUMBER;
			case LUA_TBOOLEAN:
				return *lti_BOOLEAN;
			case LUA_TSTRING:
				return *lti_STRING;
			case LUA_TTABLE:
				return *lti_TABLE;
			case LUA_TFUNCTION:
				return *lti_FUNCTION;
			case LUA_TUSERDATA:
				return *lti_USERDATA;
			case LUA_TTHREAD:
				return *lti_THREAD;
			case LUA_TLIGHTUSERDATA:
				return *lti_LIGHTUSERDATA;
			default:
				std::cerr << "lua_thread: reached unreachable control path: invalid lua type";
				std::unreachable();
			}
		}

		template<lua_like T> inline void stack::push(lua_State* state, const T& value) {
			if constexpr (nil_like<T>) {
				lua_pushnil(state);
			} else if constexpr (boolean_like<T>) {
				lua_pushboolean(state, value);
			} else if constexpr (integer_like<T>) {
				lua_pushinteger(state, value);
			} else if constexpr (number_like<T>) {
				lua_pushnumber(state, value);
			} else if constexpr (string_like<T>) {
				std::string_view sv(value);
				lua_pushlstring(state, sv.data(), sv.size());
			} else {
				never_reach("lua_thread: invalid lua type");
			}
		}

		template<lua_like T> void throw_failed_conversion(int in_type) {
			static constexpr auto fmt = "Failed to convert object of type \"{}\" to object of type \"{}\"";
			static const auto out_t_name = get_type_info(to_lua_type_v<T>).name();
			const auto in_t_name = get_type_info(in_type).name();
			throw std::invalid_argument(std::format(fmt, in_t_name, out_t_name));
		}

		template<lua_like T> inline T stack::to(lua_State* state, int pos) {
			if constexpr (nil_like<T>) {
				return nil;
			} else if constexpr (boolean_like<T>) {
				return lua_toboolean(state, pos);
			} else if constexpr (integer_like<T>) {
				int good;
				integer_t num = lua_tointegerx(state, pos, &good);
				if (!good) {
					throw_failed_conversion<T>(lua_type(state, pos));
				}
				return num;
			} else if constexpr (number_like<T>) {
				int good;
				number_t num = lua_tonumberx(state, pos, &good);
				if (!good) {
					throw_failed_conversion<T>(lua_type(state, pos));
				}
				return num;
			} else if constexpr (string_like<T>) {
				const char* str = lua_tostring(state, pos);
				if (str == nullptr) {
					throw_failed_conversion<T>(lua_type(state, pos));
				}
				return string_t {str};
			} else {
				never_reach("lua_thread: invalid lua type");
			}
		}
	} // namespace _detail

	//////////////////// Type Conversion Functions ////////////////////

	inline constexpr nil_t to_nil(const auto&) { return nil; }

	template<string_like R, class T> inline constexpr R to_string(const T& value) {
		if constexpr (nil_like<T>) {
			return "nil";
		} else if constexpr (boolean_like<T>) {
			return (value) ? "true" : "false";
		} else if constexpr (number_like<T> || integer_like<T>) {
			return std::to_string(value);
		} else if constexpr (string_like<T>) {
			return value;
		} else {
			std::cerr << "LUA_THREAD FATAL ERROR: INVALID LUA TYPE\n";
			std::unreachable();
		}
	}

	template<number_like R, class T> inline R to_number(const T& value) {
		if constexpr (nil_like<T>) {
			return 0;
		} else if constexpr (number_like<T> || boolean_like<T>) {
			return static_cast<R>(value);
		} else if constexpr (string_like<T>) {
			if constexpr (std::integral<T>) {
				return std::stoll(value);
			} else if constexpr (std::floating_point<T>) {
				return std::stod(value);
			} else {
				static_assert(false, "T is not a lua number");
			}
		} else {
			std::cerr << "LUA_THREAD FATAL ERROR: INVALID LUA TYPE\n";
			std::unreachable();
		}
	}

	template<class R, class T> inline constexpr R to_boolean(const T& value) {
		if constexpr (is_nil_v<T>) {
			return false;
		} else if constexpr (is_boolean_v<T>) {
			return value;
		} else {
			return true;
		}
	}

	//////////////////// abc ////////////////////

	template<lua_like R, lua_like... Args> R global_function<R(Args...)>::operator()(Args... args) const {
		// get the function from the registry and put it on the stack
		_detail::stack::index_registry(m_thread.internal_state(), m_registry_key);
		// then push each argument onto stack...
		(_detail::stack::push(m_thread.internal_state(), args), ...);

		// then actually call the function with the arguments
		static constexpr const int errhandler = 0;
		int errc = lua_pcall(m_thread.internal_state(), nargs, nresults, errhandler);
		if (errc != LUA_OK) {
			auto error_message = lua_tostring(m_thread.internal_state(), -1);
			_detail::throw_exception(errc, std::format("Failed to call Lua function: {}", error_message));
			lua_pop(m_thread.internal_state(), 1);
		}

		// and return the result
		R result = _detail::stack::to<R>(m_thread.internal_state(), -1);
		lua_pop(m_thread.internal_state(), 1);
		return result;
	}

	//////////////////// lua_type_info facilities ////////////////////

	template<lua_like T> const lua_type_info& lua_typeid() { return _detail::get_type_info(_detail::to_lua_type_v<T>); }

	template<lua_like T> const lua_type_info& lua_typeid(const T&) { return lua_typeid<T>(); }

	//////////////////// lua_thread ////////////////////

	template<lua_like T> T lua_thread::get_global(const std::string& name) {
		int type_id = lua_getglobal(internal_state(), name.c_str());
		if (type_id != _detail::to_lua_type_v<T>()) {
			lua_pop(internal_state(), 1);
			static constexpr const auto fmt = "Couldn't cast Lua object of type \"{}\" to object of type \"{}\")";
			static const auto t_name = _detail::get_type_info(type_id).name();
			throw std::invalid_argument(std::format(fmt, t_name, lua_typeid<T>().name()));
		}

		T value = _detail::stack::to<T>(internal_state(), -1);
		lua_pop(internal_state(), 1);
		return value;
	}

	template<lua_like R, lua_like... Args>
	global_function<R(Args...)> lua_thread::get_global_function(const std::string& name) {
		return global_function<R(Args...)> {*this, name};
	}

	template<lua_like R, lua_like... Args>
	global_function<R(Args...)>::global_function(lua_thread& thread, const std::string& fname) : m_thread {thread} {
		// push lua function to stack, then save it to the registry
		int type = lua_getglobal(m_thread.internal_state(), fname.c_str());
		if (!lua_isfunction(m_thread.internal_state(), -1)) {
			static constexpr auto fmt = "Lua object of type \"{}\" is not a function";
			static const auto t_name = _detail::get_type_info(type).name();
			throw std::invalid_argument(std::format(fmt, t_name));
		}

		assert(type == LUA_TFUNCTION);

		//
		m_registry_key = luaL_ref(m_thread.internal_state(), LUA_REGISTRYINDEX);
	}

	template<lua_like R, lua_like... Args> global_function<R(Args...)>::~global_function() {
		// remove function from registry so it can be garbage collected
		luaL_unref(m_thread.internal_state(), LUA_REGISTRYINDEX, m_registry_key);
	}
} // namespace lua

template<class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, lua::nil_t nil) {
	return os << lua::to_string(nil);
}