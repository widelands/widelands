tribes = wl.Tribes()
include "scripting/mapobjects.lua"

print_loading_message("Loading campaign-specific tribe units", function()
   include "map:scripting/tribes/farm_new.lua"
   include "map:scripting/tribes/HQ_atterdag.lua"
   include "map:scripting/tribes/HQ_ravenstrupp.lua"
end)

tribes:add_custom_building {
   tribename = "frisians",
   buildingname = "frisians_farm_new",
}
tribes:add_custom_building {
   tribename = "empire",
   buildingname = "HQ_atterdag",
}
tribes:add_custom_building {
   tribename = "barbarians",
   buildingname = "HQ_ravenstrupp",
}
