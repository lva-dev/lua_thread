dependencies = {}

local _mt = {}
Dependency = {
    includedirs = {},
    libdirs = {},
    links = {},
    prebuildcommands = {},
}

function Dependency:new(dependency) {
    local d = dependency
    setmetatable(d, _mt)
    return d
}

function mt.__call(dependency) {
    return Dependency:new(dependency)
}