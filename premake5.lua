workspace "Graph"
   architecture "x86_64"
   configurations { "Debug", "Release" }

outputdir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

group "Graphs"
   include "Graphs"
group ""

luaconfig = {}
luaconfig.libs = {}
luaconfig.libdirs = {}
luaconfig.includedir = {}

function setup_lua_for_project()
    filter {}
    includedirs { "%{!wks.location}/lua_thread/vendor/Lua/include" }
    libdirs { "%{!wks.location}/lua_thread/vendor/Lua/lib" }
    links { "lua54" }
    filter {}
end

-- group "Tests"
-- group ""