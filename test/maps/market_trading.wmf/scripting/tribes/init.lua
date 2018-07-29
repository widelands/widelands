tribes = wl.Tribes()
include "scripting/mapobjects.lua"

-- ===================================
--    Campaign Specific Data
-- ===================================
print_loading_message("Loading market unit", function()
   include "map:scripting/tribes/market/init.lua"
end)

tribes:add_custom_building {
   tribename = "barbarians",
   buildingname = "barbarians_market",
}
