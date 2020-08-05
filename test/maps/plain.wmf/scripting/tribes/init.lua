tribes = wl.Tribes()
include "scripting/mapobjects.lua"

-- ===================================
--    Campaign Specific Data
-- ===================================
print_loading_message("Loading custom units", function()
   include "map:scripting/tribes/file_animation/init.lua"
   include "map:scripting/tribes/spritesheet_animation/init.lua"
end)

tribes:add_custom_worker {
   tribename = "barbarians",
   workername = "barbarians_file_animation",
}

tribes:add_custom_building {
   tribename = "barbarians",
   buildingname = "barbarians_spritesheet_animation",
}
