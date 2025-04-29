project "main_test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    targetdir ("../bin/" .. outputdir .. "/test/%{prj.name}")
    objdir ("../bin/obj/" .. outputdir .. "/tests/%{prj.name}")

    pchheader "pch.hpp"
    pchsource "pch.cpp"

    links {
        "lua_thread",
        "lua"
    }

    includedirs {
        "../../lua_thread/include",
    }
    
    files {
        "**.cpp"
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
    