-- RST
-- mapobjects.lua
-- --------------
--
-- This script contains some convenience functions for loading the tribes
-- and world.
--

local current_time = system_time()

-- Updates current_time and prints a loading message containing the time
-- difference between old_time and current system time.
function print_loading_message(itemname)
   print("Loading ".. itemname .. " took " .. (system_time() - current_time) .. "ms")
   current_time = system_time()
end
