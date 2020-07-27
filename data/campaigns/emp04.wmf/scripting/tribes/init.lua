tribes = wl.Tribes()
include "scripting/mapobjects.lua"

push_textdomain("scenario_emp04.wmf")

-- ===================================
--    Campaign Specific Data
-- ===================================
print_loading_message(_("Loading campaign-specific tribe units"), function()
   -- ===================================
   --    Empire Mission 4
   -- ===================================
   include "map:scripting/tribes/brewery2.lua"
   include "map:scripting/tribes/brewery1.lua"
   include "map:scripting/tribes/farm2.lua"
   include "map:scripting/tribes/farm1.lua"
   include "map:scripting/tribes/foresters_house1.lua"
   include "map:scripting/tribes/lumberjacks_house1.lua"
   include "map:scripting/tribes/mill2.lua"
   include "map:scripting/tribes/mill1.lua"
   include "map:scripting/tribes/trainingcamp1.lua"
   include "map:scripting/tribes/well1.lua"
   include "map:scripting/tribes/temple_of_vesta.lua"
end)

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_brewery1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_brewery2",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_farm1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_farm2",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_foresters_house1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_lumberjacks_house1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_mill1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_mill2",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_trainingcamp1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_well1",
}

tribes:add_custom_building {
   tribename = "empire",
   buildingname = "empire_temple_of_vesta",
}

pop_textdomain()
