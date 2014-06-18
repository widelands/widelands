world = wl.World()

set_textdomain("world")

include "world/resources/init.lua"
include "world/terrains/init.lua"


world:new_editor_immovable_category{
   name = "miscellaneous",
   descname = _ "Miscellaneous",
   picture = "world/immovables/ruin5/idle.png",
}

world:new_editor_immovable_category{
   name = "plants",
   descname = _ "Plants",
   picture = "world/immovables/cactus3/idle.png",
}

world:new_editor_immovable_category{
   name = "standing_stones",
   descname = _ "Standing Stones",
   picture = "world/immovables/standing_stones/standing_stone4_desert/idle.png",
}

world:new_editor_immovable_category{
   name = "stones",
   descname = _ "Stones",
   picture = "world/immovables/stones/greenland_stones6/idle.png",
}

world:new_editor_immovable_category{
   name = "trees_coniferous",
   descname = _ "Coniferous Trees",
   picture = "world/immovables/trees/spruce_summer_old/idle_0.png",
}

world:new_editor_immovable_category{
   name = "trees_dead",
   descname = _ "Dead Trees",
   picture = "world/immovables/trees/deadtree2/idle.png",
}

world:new_editor_immovable_category{
   name = "trees_deciduous",
   descname = _ "Deciduous Trees",
   picture = "world/immovables/trees/alder_summer_old/idle_0.png",
}

world:new_editor_immovable_category{
   name = "trees_palm",
   descname = _ "Palm Trees",
   picture = "world/immovables/trees/palm_borassus_desert_old/idle_0.png",
}

world:new_editor_immovable_category{
   name = "trees_wasteland",
   descname = _ "Wasteland Trees",
   picture = "world/immovables/trees/umbrella_red_wasteland_old/idle_0.png",
}

include "world/immovables/grass1/init.lua"
include "world/immovables/bush1/init.lua"
include "world/immovables/bush2/init.lua"
include "world/immovables/bush3/init.lua"
include "world/immovables/bush4/init.lua"
include "world/immovables/bush5/init.lua"
include "world/immovables/cactus1/init.lua"
include "world/immovables/cactus2/init.lua"
include "world/immovables/cactus3/init.lua"
include "world/immovables/cactus4/init.lua"
include "world/immovables/grass2/init.lua"
include "world/immovables/grass3/init.lua"
include "world/immovables/manmade/bar-ruin00/init.lua"
include "world/immovables/manmade/bar-ruin01/init.lua"
include "world/immovables/manmade/bar-ruin02/init.lua"
include "world/immovables/manmade/bar-ruin03/init.lua"
include "world/immovables/manmade/debris00/init.lua"
include "world/immovables/manmade/debris01/init.lua"
include "world/immovables/manmade/debris02/init.lua"
include "world/immovables/manmade/snowman/init.lua"
include "world/immovables/mushroom1/init.lua"
include "world/immovables/mushroom2/init.lua"
include "world/immovables/pebble1/init.lua"
include "world/immovables/pebble2/init.lua"
include "world/immovables/pebble3/init.lua"
include "world/immovables/pebble4/init.lua"
include "world/immovables/pebble5/init.lua"
include "world/immovables/pebble6/init.lua"
include "world/immovables/ruin1/init.lua"
include "world/immovables/ruin2/init.lua"
include "world/immovables/ruin3/init.lua"
include "world/immovables/ruin4/init.lua"
include "world/immovables/ruin5/init.lua"
include "world/immovables/skeleton1/init.lua"
include "world/immovables/skeleton2/init.lua"
include "world/immovables/skeleton3/init.lua"
include "world/immovables/skeleton4/init.lua"
include "world/immovables/track_winter/init.lua"

-- Standing Stones
include "world/immovables/standing_stones/standing_stone1_desert/init.lua"
include "world/immovables/standing_stones/standing_stone1_summer/init.lua"
include "world/immovables/standing_stones/standing_stone1_wasteland/init.lua"
include "world/immovables/standing_stones/standing_stone1_winter/init.lua"
include "world/immovables/standing_stones/standing_stone2_desert/init.lua"
include "world/immovables/standing_stones/standing_stone2_summer/init.lua"
include "world/immovables/standing_stones/standing_stone2_wasteland/init.lua"
include "world/immovables/standing_stones/standing_stone2_winter/init.lua"
include "world/immovables/standing_stones/standing_stone3_desert/init.lua"
include "world/immovables/standing_stones/standing_stone3_summer/init.lua"
include "world/immovables/standing_stones/standing_stone3_wasteland/init.lua"
include "world/immovables/standing_stones/standing_stone3_winter/init.lua"
include "world/immovables/standing_stones/standing_stone4_desert/init.lua"
include "world/immovables/standing_stones/standing_stone4_summer/init.lua"
include "world/immovables/standing_stones/standing_stone4_wasteland/init.lua"
include "world/immovables/standing_stones/standing_stone4_winter/init.lua"
include "world/immovables/standing_stones/standing_stone5_desert/init.lua"
include "world/immovables/standing_stones/standing_stone5_summer/init.lua"
include "world/immovables/standing_stones/standing_stone5_wasteland/init.lua"
include "world/immovables/standing_stones/standing_stone5_winter/init.lua"
include "world/immovables/standing_stones/standing_stone6/init.lua"
include "world/immovables/standing_stones/standing_stone7/init.lua"

-- Stones
include "world/immovables/stones/blackland_stones1/init.lua"
include "world/immovables/stones/blackland_stones2/init.lua"
include "world/immovables/stones/blackland_stones3/init.lua"
include "world/immovables/stones/blackland_stones4/init.lua"
include "world/immovables/stones/blackland_stones5/init.lua"
include "world/immovables/stones/blackland_stones6/init.lua"
include "world/immovables/stones/desert_stones1/init.lua"
include "world/immovables/stones/desert_stones2/init.lua"
include "world/immovables/stones/desert_stones3/init.lua"
include "world/immovables/stones/desert_stones4/init.lua"
include "world/immovables/stones/desert_stones5/init.lua"
include "world/immovables/stones/desert_stones6/init.lua"
include "world/immovables/stones/greenland_stones1/init.lua"
include "world/immovables/stones/greenland_stones2/init.lua"
include "world/immovables/stones/greenland_stones3/init.lua"
include "world/immovables/stones/greenland_stones4/init.lua"
include "world/immovables/stones/greenland_stones5/init.lua"
include "world/immovables/stones/greenland_stones6/init.lua"
include "world/immovables/stones/winterland_stones1/init.lua"
include "world/immovables/stones/winterland_stones2/init.lua"
include "world/immovables/stones/winterland_stones3/init.lua"
include "world/immovables/stones/winterland_stones4/init.lua"
include "world/immovables/stones/winterland_stones5/init.lua"
include "world/immovables/stones/winterland_stones6/init.lua"

-- Trees
include "world/immovables/trees/alder_summer_mature/init.lua"
include "world/immovables/trees/alder_summer_old/init.lua"
include "world/immovables/trees/alder_summer_pole/init.lua"
include "world/immovables/trees/alder_summer_sapling/init.lua"

include "world/immovables/trees/aspen_summer_mature/init.lua"
include "world/immovables/trees/aspen_summer_old/init.lua"
include "world/immovables/trees/aspen_summer_pole/init.lua"
include "world/immovables/trees/aspen_summer_sapling/init.lua"

include "world/immovables/trees/beech_summer_mature/init.lua"
include "world/immovables/trees/beech_summer_old/init.lua"
include "world/immovables/trees/beech_summer_pole/init.lua"
include "world/immovables/trees/beech_summer_sapling/init.lua"

include "world/immovables/trees/birch_summer_mature/init.lua"
include "world/immovables/trees/birch_summer_old/init.lua"
include "world/immovables/trees/birch_summer_pole/init.lua"
include "world/immovables/trees/birch_summer_sapling/init.lua"

include "world/immovables/trees/cirrus_wasteland_mature/init.lua"
include "world/immovables/trees/cirrus_wasteland_old/init.lua"
include "world/immovables/trees/cirrus_wasteland_pole/init.lua"
include "world/immovables/trees/cirrus_wasteland_sapling/init.lua"

include "world/immovables/trees/deadtree1/init.lua"
include "world/immovables/trees/deadtree2/init.lua"
include "world/immovables/trees/deadtree3/init.lua"
include "world/immovables/trees/deadtree4/init.lua"
include "world/immovables/trees/deadtree5/init.lua"
include "world/immovables/trees/deadtree6/init.lua"

include "world/immovables/trees/fallentree/init.lua"

include "world/immovables/trees/larch_summer_mature/init.lua"
include "world/immovables/trees/larch_summer_old/init.lua"
include "world/immovables/trees/larch_summer_pole/init.lua"
include "world/immovables/trees/larch_summer_sapling/init.lua"

include "world/immovables/trees/liana_wasteland_mature/init.lua"
include "world/immovables/trees/liana_wasteland_old/init.lua"
include "world/immovables/trees/liana_wasteland_pole/init.lua"
include "world/immovables/trees/liana_wasteland_sapling/init.lua"

include "world/immovables/trees/maple_winter_mature/init.lua"
include "world/immovables/trees/maple_winter_old/init.lua"
include "world/immovables/trees/maple_winter_pole/init.lua"
include "world/immovables/trees/maple_winter_sapling/init.lua"

include "world/immovables/trees/mushroom_dark_wasteland_mature/init.lua"
include "world/immovables/trees/mushroom_dark_wasteland_old/init.lua"
include "world/immovables/trees/mushroom_dark_wasteland_pole/init.lua"
include "world/immovables/trees/mushroom_dark_wasteland_sapling/init.lua"

include "world/immovables/trees/mushroom_green_wasteland_mature/init.lua"
include "world/immovables/trees/mushroom_green_wasteland_old/init.lua"
include "world/immovables/trees/mushroom_green_wasteland_pole/init.lua"
include "world/immovables/trees/mushroom_green_wasteland_sapling/init.lua"

include "world/immovables/trees/mushroom_red_wasteland_mature/init.lua"
include "world/immovables/trees/mushroom_red_wasteland_old/init.lua"
include "world/immovables/trees/mushroom_red_wasteland_pole/init.lua"
include "world/immovables/trees/mushroom_red_wasteland_sapling/init.lua"

include "world/immovables/trees/oak_summer_mature/init.lua"
include "world/immovables/trees/oak_summer_old/init.lua"
include "world/immovables/trees/oak_summer_pole/init.lua"
include "world/immovables/trees/oak_summer_sapling/init.lua"

include "world/immovables/trees/palm_borassus_desert_mature/init.lua"
include "world/immovables/trees/palm_borassus_desert_old/init.lua"
include "world/immovables/trees/palm_borassus_desert_pole/init.lua"
include "world/immovables/trees/palm_borassus_desert_sapling/init.lua"

include "world/immovables/trees/palm_coconut_desert_mature/init.lua"
include "world/immovables/trees/palm_coconut_desert_old/init.lua"
include "world/immovables/trees/palm_coconut_desert_pole/init.lua"
include "world/immovables/trees/palm_coconut_desert_sapling/init.lua"

include "world/immovables/trees/palm_date_desert_mature/init.lua"
include "world/immovables/trees/palm_date_desert_old/init.lua"
include "world/immovables/trees/palm_date_desert_pole/init.lua"
include "world/immovables/trees/palm_date_desert_sapling/init.lua"

include "world/immovables/trees/palm_oil_desert_mature/init.lua"
include "world/immovables/trees/palm_oil_desert_old/init.lua"
include "world/immovables/trees/palm_oil_desert_pole/init.lua"
include "world/immovables/trees/palm_oil_desert_sapling/init.lua"

include "world/immovables/trees/palm_roystonea_desert_mature/init.lua"
include "world/immovables/trees/palm_roystonea_desert_old/init.lua"
include "world/immovables/trees/palm_roystonea_desert_pole/init.lua"
include "world/immovables/trees/palm_roystonea_desert_sapling/init.lua"

include "world/immovables/trees/rowan_summer_mature/init.lua"
include "world/immovables/trees/rowan_summer_old/init.lua"
include "world/immovables/trees/rowan_summer_pole/init.lua"
include "world/immovables/trees/rowan_summer_sapling/init.lua"

include "world/immovables/trees/spruce_summer_mature/init.lua"
include "world/immovables/trees/spruce_summer_old/init.lua"
include "world/immovables/trees/spruce_summer_pole/init.lua"
include "world/immovables/trees/spruce_summer_sapling/init.lua"

include "world/immovables/trees/twine_wasteland_mature/init.lua"
include "world/immovables/trees/twine_wasteland_old/init.lua"
include "world/immovables/trees/twine_wasteland_pole/init.lua"
include "world/immovables/trees/twine_wasteland_sapling/init.lua"

include "world/immovables/trees/umbrella_red_wasteland_mature/init.lua"
include "world/immovables/trees/umbrella_red_wasteland_old/init.lua"
include "world/immovables/trees/umbrella_red_wasteland_pole/init.lua"
include "world/immovables/trees/umbrella_red_wasteland_sapling/init.lua"

include "world/immovables/trees/umbrella_green_wasteland_mature/init.lua"
include "world/immovables/trees/umbrella_green_wasteland_old/init.lua"
include "world/immovables/trees/umbrella_green_wasteland_pole/init.lua"
include "world/immovables/trees/umbrella_green_wasteland_sapling/init.lua"


-- Adds 6 animations for each walking direction into 'table'. The pictures are
-- searched for in 'dirname'. All files should look like this
-- 'basename'_(e|ne|se|sw|w|nw)_\d+.png. 'hotspot' is the hotspot for blitting.
function add_walking_animations(table, dirname, basename, hotspot, fps)
   for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
      table["walk_" .. dir] = {
         pictures = path.list_directory(dirname, basename .. "_" .. dir .. "_\\d+.png"),
         hotspot = hotspot,
         fps = fps,
      }
   end
end

include "world/critters/badger/init.lua"
include "world/critters/brownbear/init.lua"
include "world/critters/bunny/init.lua"
include "world/critters/chamois/init.lua"
include "world/critters/deer/init.lua"
include "world/critters/duck/init.lua"
include "world/critters/elk/init.lua"
include "world/critters/fox/init.lua"
include "world/critters/lynx/init.lua"
include "world/critters/marten/init.lua"
include "world/critters/reindeer/init.lua"
include "world/critters/sheep/init.lua"
include "world/critters/stag/init.lua"
include "world/critters/wisent/init.lua"
include "world/critters/wolf/init.lua"
