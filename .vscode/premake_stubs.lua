-- premake_stubs.lua

---@diagnostic disable: lowercase-global

function workspace(name) end
function project(name) end
function location(path) end
function kind(name) end
function language(name) end
function files(patterns) end
function filter(criteria) end
function configurations(list) end
function includedirs(list) end
function defines(list) end
function links(list) end
function targetdir(dir) end
function targetname(name) end

function buildmessage(message) end
function buildcommands(commands) end
function buildoutputs(s) end
function rebuildcommands(s) end
function cleancommands(s) end

---@diagnostic enable: lowercase-global