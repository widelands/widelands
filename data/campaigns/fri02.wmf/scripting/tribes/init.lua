tribes = wl.Tribes()
include "scripting/mapobjects.lua"

print_loading_message("Loading campaign-specific tribe units", function()
   include "map:scripting/tribes/warehouse_empire.lua"
end)

tribes:add_custom_building {
   tribename = "frisians",
   buildingname = "frisians_warehouse_empire",
}
