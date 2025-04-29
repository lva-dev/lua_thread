workspace "lua_thread"
   architecture "x86_64"
   configurations { "Debug", "Release" }

outputdir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

group "lua_thread"
   include "lua_thread"
group ""

group "Tests"
   include "tests/main_test"
group ""