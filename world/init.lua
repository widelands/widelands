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
   name = "rocks",
   descname = _ "Rocks",
   picture = "world/immovables/rocks/greenland_rocks6/idle.png",
}

world:new_editor_immovable_category{
   name = "trees_coniferous",
   descname = _ "Coniferous Trees",
   picture = "world/immovables/trees/spruce/old/idle_0.png",
}

world:new_editor_immovable_category{
   name = "trees_dead",
   descname = _ "Dead Trees",
   picture = "world/immovables/trees/deadtree2/idle.png",
}

world:new_editor_immovable_category{
   name = "trees_deciduous",
   descname = _ "Deciduous Trees",
   picture = "world/immovables/trees/alder/old/idle_0.png",
}

world:new_editor_immovable_category{
   name = "trees_palm",
   descname = _ "Palm Trees",
   picture = "world/immovables/trees/palm_borassus/old/idle_0.png",
}

world:new_editor_immovable_category{
   name = "trees_wasteland",
   descname = _ "Wasteland Trees",
   picture = "world/immovables/trees/umbrella_red/old/idle_0.png",
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

-- Rocks
include "world/immovables/rocks/blackland_rocks1/init.lua"
include "world/immovables/rocks/blackland_rocks2/init.lua"
include "world/immovables/rocks/blackland_rocks3/init.lua"
include "world/immovables/rocks/blackland_rocks4/init.lua"
include "world/immovables/rocks/blackland_rocks5/init.lua"
include "world/immovables/rocks/blackland_rocks6/init.lua"
include "world/immovables/rocks/desert_rocks1/init.lua"
include "world/immovables/rocks/desert_rocks2/init.lua"
include "world/immovables/rocks/desert_rocks3/init.lua"
include "world/immovables/rocks/desert_rocks4/init.lua"
include "world/immovables/rocks/desert_rocks5/init.lua"
include "world/immovables/rocks/desert_rocks6/init.lua"
include "world/immovables/rocks/greenland_rocks1/init.lua"
include "world/immovables/rocks/greenland_rocks2/init.lua"
include "world/immovables/rocks/greenland_rocks3/init.lua"
include "world/immovables/rocks/greenland_rocks4/init.lua"
include "world/immovables/rocks/greenland_rocks5/init.lua"
include "world/immovables/rocks/greenland_rocks6/init.lua"
include "world/immovables/rocks/winterland_rocks1/init.lua"
include "world/immovables/rocks/winterland_rocks2/init.lua"
include "world/immovables/rocks/winterland_rocks3/init.lua"
include "world/immovables/rocks/winterland_rocks4/init.lua"
include "world/immovables/rocks/winterland_rocks5/init.lua"
include "world/immovables/rocks/winterland_rocks6/init.lua"

-- Trees
include "world/immovables/trees/alder/init.lua"
include "world/immovables/trees/aspen/init.lua"
include "world/immovables/trees/beech/init.lua"
include "world/immovables/trees/birch/init.lua"
include "world/immovables/trees/cirrus/init.lua"
include "world/immovables/trees/deadtree1/init.lua"
include "world/immovables/trees/deadtree2/init.lua"
include "world/immovables/trees/deadtree3/init.lua"
include "world/immovables/trees/deadtree4/init.lua"
include "world/immovables/trees/deadtree5/init.lua"
include "world/immovables/trees/deadtree6/init.lua"
include "world/immovables/trees/fallentree/init.lua"
include "world/immovables/trees/larch/init.lua"
include "world/immovables/trees/liana/init.lua"
include "world/immovables/trees/maple/init.lua"
include "world/immovables/trees/mushroom_dark/init.lua"
include "world/immovables/trees/mushroom_green/init.lua"
include "world/immovables/trees/mushroom_red/init.lua"
include "world/immovables/trees/oak/init.lua"
include "world/immovables/trees/palm_borassus/init.lua"
include "world/immovables/trees/palm_coconut/init.lua"
include "world/immovables/trees/palm_date/init.lua"
include "world/immovables/trees/palm_oil/init.lua"
include "world/immovables/trees/palm_roystonea/init.lua"
include "world/immovables/trees/rowan/init.lua"
include "world/immovables/trees/spruce/init.lua"
include "world/immovables/trees/twine/init.lua"
include "world/immovables/trees/umbrella_green/init.lua"
include "world/immovables/trees/umbrella_red/init.lua"

-- Adds 6 animations for each walking direction into 'table'. The pictures are
-- searched for in 'dirname'. All files should look like this
-- 'basename'_(e|ne|se|sw|w|nw)_\d+.png. 'hotspot' is the hotspot for blitting.
function add_walking_animations(table, dirname, basename, hotspot, fps)
   for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
      table["walk_" .. dir] = {
         pictures = path.list_files(dirname .. basename .. "_" .. dir ..  "_??.png"),
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
include "world/critters/wildboar/init.lua"
include "world/critters/wisent/init.lua"
include "world/critters/wolf/init.lua"
