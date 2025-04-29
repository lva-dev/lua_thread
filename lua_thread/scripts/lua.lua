function SetupLuaForProject()
    local system_xxx = {}
    system_xxx["aix"] = "aix"
    system_xxx["bsd"] = "freebsd"
    system_xxx["ios"] = "ios"
    system_xxx["linux"] = "linux"
    system_xxx["macosx"] = "macosx"
    system_xxx["solaris"] = "solaris"
    system_xxx["windows"] = "mingw"

    local dep = {}
    dep.lua = {}
    dep.lua.luaname = "lua-5.4.7"
    dep.lua.rootpath = "%{!wks.location}/lua_thread/vendor/lua-5.4.7"
    dep.lua.installpath = dep.lua.rootpath .. "/install"

    dep.lua.outpath = "%{!wks.location}/bin/" .. outputdir .. "/lua_thread"
    dep.lua.outincludedirs = { dep.lua.installpath .. "/include" }
    dep.lua.outlibdirs = { dep.lua.installpath .. "/lib" }
    dep.lua.outlinks = { "lua" }

    dep.lua.system = system_xxx["%{cfg.system}"]
    dep.lua.system = "mingw"
    
    -- Building lua
    -- buildmessage ("building " .. dep.lua.luaname .. "...")

    prebuildcommands {
        "pushd " .. dep.lua.rootpath .. " && make " .. dep.lua.system .. " && make install INSTALL_TOP=" .. dep.lua.installpath .. " && popd"
    }

    
    prebuildcommands {
        "cp " .. dep.lua.installpath .. "/lib/liblua.a " .. dep.lua.outpath
    }

    -- rebuildcommands {
    --     "pushd " .. dep.lua.rootpath .. " && make " .. dep.lua.system .. " && make install INSTALL_TOP=" .. dep.lua.outputpath .. " && popd"
    -- }

    -- cleancommands {
    --     "pushd " .. dep.lua.rootpath .. " make clean " .. dep.lua.system
    -- }
    
    -- For project
    includedirs { table.unpack(dep.lua.outincludedirs) }
    libdirs { table.unpack(dep.lua.outlibdirs) }
    links { table.unpack(dep.lua.outlinks) }
end