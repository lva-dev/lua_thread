#include "pch.hpp"

#include <lua_thread/lua_thread.hpp>

#include <lua.h>

#include <exception>
#include <format>
#include <future>
#include <system_error>
#include <thread>
#include <utility>

namespace lua {
	namespace _detail {
		/**
		 * Creates a new `lua_State`.
		 *
		 * Throws `memory_alloc_error` on failure.
		 * a pointer to a newly created lua_State
		 */
		static lua_State* new_state() {
			lua_State* state = luaL_newstate();
			if (state == nullptr) {
				throw memory_alloc_error("Failed to create a Lua state: Not enough memory");
			}

			return state;
		}

		/**
		 * Creates a new `lua_State` with all of the standard Lua libraries.
		 *
		 * Throws `memory_alloc_error` on failure.
		 * @return a pointer to a newly created lua_State
		 */
		static lua_State* new_state_with_libs() {
			lua_State* state = new_state();
			luaL_openlibs(state);
			return state;
		}

		std::unique_ptr<lua_State, void (*)(lua_State*)> global_lua_state {new_state(), lua_close};

		/**
		 */
		int stack::index_registry(lua_State* state, int key) {
            lua_pushinteger(state, key);
            return lua_gettable(state, LUA_REGISTRYINDEX);
        }
	} // namespace _detail

	//////////////////////// lua_type_info ////////////////////////

	const char* lua_type_info::name() const { return m_type_name; }

	int lua_type_info::dynamic_type() const { return m_lua_type_id; }

	lua_type_info::lua_type_info(lua_State* state, int lua_type_id)
		: m_lua_type_id {lua_type_id},
		  m_state {state},
		  m_type_name(lua_typename(m_state, m_lua_type_id)) {}

	//////////////////////// lua_thread::id ////////////////////////

	lua_thread::id::id(id&& other) noexcept : m_id(std::exchange(other.m_id, reinterpret_cast<std::size_t>(nullptr))) {}

	lua_thread::id& lua_thread::id::operator=(id&& other) noexcept {
		m_id = std::exchange(other.m_id, reinterpret_cast<std::size_t>(nullptr));
		return *this;
	}

	lua_thread::id::id(const lua_thread& thread) : m_id {reinterpret_cast<std::size_t>(thread.internal_state())} {}

	bool lua_thread::id::operator==(id other) const noexcept { return m_id == other.m_id; }

	std::strong_ordering lua_thread::id::operator<=>(id other) const noexcept { return m_id <=> other.m_id; }

	//////////////////////// lua_thread ////////////////////////

	lua_thread::lua_thread(lua_thread&& other) { this->m_state = std::exchange(other.m_state, nullptr); }

	lua_thread& lua_thread::operator=(lua_thread&& other) {
		if (this->joinable()) {
			std::terminate();
		}

		this->m_state = std::exchange(other.m_state, nullptr);
		return *this;
	}

	lua_thread lua_thread::new_initialized(launch launch_policy) {
		lua_thread new_thread;
		new_thread.m_state = _detail::new_state_with_libs();
		new_thread.m_launch_policy = launch_policy;
		return new_thread;
	}

	lua_thread lua_thread::from_file(launch policy, const std::filesystem::path& file) {
		lua_thread new_thread = new_initialized(policy);
		new_thread.load_file_as_chunk(file);

		if (policy == launch::run) {
			new_thread.m_running_thread = std::thread(&lua_thread::pcall_chunk, &new_thread);
		} else if (policy == launch::load) {
			new_thread.pcall_chunk();

			// if (!lua_setupvalue(new_thread.internal_state(), -1, 1)) {
			//     std::cout << "problem";
			// }
		}

		return new_thread;
	}

	lua_thread lua_thread::from_string(launch policy, const std::string& source) {
		lua_thread new_thread = new_initialized(policy);
		new_thread.load_string_as_chunk(source);

		if (policy == launch::run) {
			new_thread.m_running_thread = std::thread(&lua_thread::pcall_chunk, &new_thread);
		} else if (policy == launch::load) {
			new_thread.pcall_chunk();

			// if (!lua_setupvalue(new_thread.internal_state(), -1, 1)) {
			//     std::cout << "problem";
			// }
		}

		return new_thread;
	}

	lua_thread::~lua_thread() {
		if (joinable()) {
			std::terminate();
		}

		if (m_state != nullptr) {
			lua_close(m_state);
		}
	}

	bool lua_thread::joinable() const noexcept { return get_id() != id {}; }

	void lua_thread::join() {
		if (!joinable()) {
			throw std::system_error(std::make_error_code(std::errc::invalid_argument));
		}

		if (m_launch_policy == launch::run) {
			m_running_thread.join();
		}

		lua_close(m_state);
		m_state = nullptr;
	}

	void lua_thread::detatch() noexcept { get_id() = id {}; }

	lua_thread::id lua_thread::get_id() const noexcept { return id {*this}; }

	lua_State* lua_thread::internal_state() const noexcept { return m_state; }

	void lua_thread::swap(lua_thread& other) { std::swap(this->m_state, other.m_state); }

	void lua_thread::load_file_as_chunk(const std::filesystem::path& file) {
		std::string file_str = file.string();
		int loadfile_errc = luaL_loadfile(internal_state(), file_str.c_str());
		if (loadfile_errc != LUA_OK) {
			static constexpr const char* fmt = "lua_thread error: Failed to load file from \"{}\"";
			_detail::throw_exception(loadfile_errc, std::format(fmt, __func__, file.string()));
		}
	}

	void lua_thread::load_string_as_chunk(const std::string& source) {
		int loadstring_errc = luaL_loadstring(internal_state(), source.c_str());
		if (loadstring_errc != LUA_OK) {
			static constexpr const char* fmt = "lua_thread error: Failed to load script from string";
			_detail::throw_exception(loadstring_errc, std::format(fmt, __func__, source));
		}
	}

	void lua_thread::pcall_chunk() {
		static constexpr const int N_ARGUMENTS = 0;
		static constexpr const int N_RESULTS = LUA_MULTRET;
		static constexpr const int ERR_HANDLER = 0;
		static constexpr const lua_KContext CTX = 0;
		static constexpr const lua_KFunction K = nullptr;

		int errc = lua_pcallk(internal_state(), N_ARGUMENTS, N_RESULTS, ERR_HANDLER, CTX, K);
		if (errc != LUA_OK) {
			_detail::throw_exception(errc, "lua_thread error: Failed to call Lua function");
		}
	}
}; // namespace lua