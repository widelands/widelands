if player1 then -- was loaded before
   -- avoid running tests twice, which happens in editor
   wl.ui.MapView():close() -- was done by init.lua, but must be repeated
   return
end

-- "map:scripting/common_init.lua" is included by "map:scripting/init.lua"
include "map:scripting/init.lua" -- Run the ordinary testsuite
