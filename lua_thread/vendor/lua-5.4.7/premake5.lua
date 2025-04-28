project "lua"
    kind "Makefile"

    buildcommands {
        "make %{cfg.buildcfg}"
    }

    rebuildcommands {
        "make %{cfg.buildcfg} rebuild"
    }

    cleancommands {
        "make clean %{cfg.buildcfg}"
    }
     
includedirs { "!%{!wks.location}/lua_thread/vendor/lua-5.4.7/include" }
libdirs { "%{!wks.location}/lua_thread/vendor/lua-5.4.7/lib" }
links { "lua54" }

function setup_lua_in_project() {
    filter {}
    buildcommands {
        "make mingw && make install local"
    }

    rebuildcommands {
        "make mingw && make install local"
    }

    cleancommands {
        "make clean mingw"
    }
    
    includedirs { "!%{!wks.location}/lua_thread/vendor/lua-5.4.7/include" }
    libdirs { "%{!wks.location}/lua_thread/vendor/lua-5.4.7/lib" }
    links { "lua54" }
    filter {}
}