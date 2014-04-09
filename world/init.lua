world = wl.World()

world:new_editor_category{
   name = "green",
   descname = _ "Summer",
   picture = "world/pics/editor_category_green.png",
}
world:new_editor_category{
   name = "wasteland",
   descname = _ "Wasteland",
   picture = "world/pics/editor_category_wasteland.png",
}
world:new_editor_category{
   name = "winter",
   descname = _ "Winter",
   picture = "world/pics/editor_category_winter.png",
}
world:new_editor_category{
   name = "desert",
   descname = _ "Desert",
   picture = "world/pics/editor_category_desert.png",
}

include "world/resources/init.lua"
include "world/terrains/init.lua"

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

-- Standing Stones
include "world/immovables/sstones1/init.lua"
include "world/immovables/sstones10/init.lua"
include "world/immovables/sstones11/init.lua"
include "world/immovables/sstones12/init.lua"
include "world/immovables/sstones13/init.lua"
include "world/immovables/sstones14/init.lua"
include "world/immovables/sstones15/init.lua"
include "world/immovables/sstones16/init.lua"
include "world/immovables/sstones17/init.lua"
include "world/immovables/sstones18/init.lua"
include "world/immovables/sstones19/init.lua"
include "world/immovables/sstones2/init.lua"
include "world/immovables/sstones20/init.lua"
include "world/immovables/sstones21/init.lua"
include "world/immovables/sstones22/init.lua"
include "world/immovables/sstones3/init.lua"
include "world/immovables/sstones4/init.lua"
include "world/immovables/sstones5/init.lua"
include "world/immovables/sstones6/init.lua"
include "world/immovables/sstones7/init.lua"
include "world/immovables/sstones8/init.lua"
include "world/immovables/sstones9/init.lua"

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
include "world/immovables/trees/larch_summer_mature/init.lua"
include "world/immovables/trees/larch_summer_old/init.lua"
include "world/immovables/trees/larch_summer_pole/init.lua"
include "world/immovables/trees/larch_summer_sapling/init.lua"
include "world/immovables/trees/oak_summer_mature/init.lua"
include "world/immovables/trees/oak_summer_old/init.lua"
include "world/immovables/trees/oak_summer_pole/init.lua"
include "world/immovables/trees/oak_summer_sapling/init.lua"
include "world/immovables/trees/rowan_summer_mature/init.lua"
include "world/immovables/trees/rowan_summer_old/init.lua"
include "world/immovables/trees/rowan_summer_pole/init.lua"
include "world/immovables/trees/rowan_summer_sapling/init.lua"
include "world/immovables/trees/spruce_summer_mature/init.lua"
include "world/immovables/trees/spruce_summer_old/init.lua"
include "world/immovables/trees/spruce_summer_pole/init.lua"
include "world/immovables/trees/spruce_summer_sapling/init.lua"
include "world/immovables/trees/deadtree1/init.lua"
include "world/immovables/trees/deadtree2/init.lua"
include "world/immovables/trees/deadtree3/init.lua"
include "world/immovables/trees/deadtree4/init.lua"
include "world/immovables/trees/deadtree5/init.lua"
include "world/immovables/trees/deadtree6/init.lua"
include "world/immovables/trees/fallentree/init.lua"


-- Adds 6 animations for each walking direction into 'table'. The pictures are
-- searched for in 'dirname'. All files should look like this
-- 'basename'_(e|ne|se|sw|w|nw)_\d+.png. 'hotspot' is the hotspot for blitting.
function add_walking_animations(table, dirname, basename, hotspot, fps)
   for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
      table["walk_" .. dir] = {
         pictures = path.glob(dirname, basename .. "_" .. dir .. "_\\d+.png"),
         player_color_masks = {},
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
