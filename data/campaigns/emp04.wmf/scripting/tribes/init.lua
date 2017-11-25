tribes = wl.Tribes()
include "scripting/mapobjects.lua"

-- ===================================
--    Campaign Specific Data
-- ===================================
print_loading_message("Loading campaign-specific tribe units", function()
   -- ===================================
   --    Empire Mission 4
   -- ===================================
   include "map:scripting/tribes/brewery.lua"
   include "map:scripting/tribes/farm.lua"
   include "map:scripting/tribes/foresters_house1.lua"
   include "map:scripting/tribes/foresters_house.lua"
   include "map:scripting/tribes/lumberjacks_house1.lua"
   include "map:scripting/tribes/lumberjacks_house.lua"
   include "map:scripting/tribes/mill.lua"
   include "map:scripting/tribes/trainingcamp.lua"
   include "map:scripting/tribes/well1.lua"
   include "map:scripting/tribes/well.lua"
end)

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_brewery1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_farm1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_foresters_house2",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_lumberjacks_house2",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_mill1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_trainingcamp1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_well2",
}
