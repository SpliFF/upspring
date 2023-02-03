---------------------------------
-- HEADER: Each Script needs this
---------------------------------
local info = debug.getinfo(1,'S');
script_path = info.source:match[[^@?(.*[\/])[^\/]-$]]
package.path = package.path .. ";" .. script_path .. "?/init.lua" .. ";" .. script_path .. "?.lua"

lib = require("lib")

LoadArchives()

---------------------------------
-- Actual code
---------------------------------
io.write(".3do file to import: ")
local input = io.read()

if not lib.utils.is_file(input) then
    error("ERROR: File '" .. input .. "' doesn't exist.")
end

lib.model.Convert3DOToS3O(input)