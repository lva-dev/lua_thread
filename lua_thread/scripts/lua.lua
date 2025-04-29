
local function get_lua_makefile_system()
    if     os.target() == "aix"      then return "aix"
    elseif os.target() == "bsd"      then return "freebsd"
    elseif os.target() == "ios"      then return "ios"
    elseif os.target() == "linux"    then return "linux"
    elseif os.target() == "macosx"   then return "macosx"
    elseif os.target() == "solaris"  then return "solaris"
    elseif os.target() == "windows"  then return "mingw"
    else return "guess"
    end
end

function SetupLuaForProject()
    local dep = {}
    dep.lua = {}
    
    dep.lua.name        = "lua-5.4.7"
    dep.lua.rootpath    = "%{!wks.location}/lua_thread/vendor/lua-5.4.7"
    dep.lua.installpath = path.join("%{!wks.location}/bin/", outputdir, "/lua")
    dep.lua.outpath     = path.join("%{!wks.location}/bin/", outputdir, "/lua")

    dep.lua.outincludedirs  = { path.join(dep.lua.installpath, "/include") }
    dep.lua.outlibdirs      = { path.join(dep.lua.installpath, "/lib") }
    dep.lua.outlinks        = { "lua" }
    
    dep.lua.system = get_lua_makefile_system()

    -- Building lua
    -- buildmessage ("building " .. dep.lua.name .. "...")

    prebuildcommands {
        "pushd " .. dep.lua.rootpath .. " && make " .. dep.lua.system .. " && make install INSTALL_TOP=" .. dep.lua.installpath .. " && popd",
        "cp " .. dep.lua.installpath .. "/lib/liblua.a " .. dep.lua.outpath
    }
    
    prebuildcommands {}
    
    -- For project
    includedirs { dep.lua.outincludedirs }
    libdirs { dep.lua.outlibdirs }
    links { dep.lua.outlinks }
end