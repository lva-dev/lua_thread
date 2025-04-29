include "scripts/lua.lua"

project "lua_thread"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../../bin/obj/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.hpp"
    pchsource "pch.cpp"

    SetupLuaForProject()

    includedirs {
        "include",
        "src"
    }
    
    files {
        "include/**.h",
        "include/**.hpp",
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-Wall",
            "-Wno-unused"
        }

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"
    
    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"
    