

local function get_system_for_makefile()
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

local lua_system        = get_system_for_makefile()
local lua_rootpath      = "%{!wks.location}/lua_thread/vendor/lua-5.4.7"
local lua_installpath   = path.join(lua_rootpath, "install", lua_system)
local lua_includedir    = path.join(lua_installpath, "include");
local lua_libdir        = path.join(lua_installpath, "lib");

dependencies.lua = {}
dependencies.lua.includedirs  = { lua_includedir }
dependencies.lua.libdirs      = { lua_libdir }
dependencies.lua.links        = { "lua" }
dependencies.lua.prebuildcommands = string.concat({
    "{ECHO} \"Building lua...\""
    "pushd " .. lua_rootpath,
    "make " .. lua_system,
    "make install INSTALL_TOP=" .. lua_installpath, 
    "popd"
    "{ECHO} \"Lua built successfully.\"", " && "})

function SetupLuaForProject()
    -- For project
    prebuildcommands dependencies.lua.prebuildcommands
    includedirs dependencies.lua.includedirs
    libdirs dependencies.lua.libdirs
    links dependencies.lua.links
end