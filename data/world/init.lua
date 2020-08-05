-- RST
-- init.lua
-- --------
--
-- World initialization.
-- All world entities are loaded via this file.
--
-- This file also defines the editor categories for world elements like terrains or
-- immovables so that they will be added to their respective editor tools (Place Terrain,
-- Place Immovable etc.). There are three categories available,
-- each with their own function:

world = wl.World()

if wl.Game then egbase = wl.Game() else egbase = wl.Editor() end
function set_loading_message(str, i)
   egbase:set_loading_message(_("Loading world: %1$s (%2$d/%3$d)"):bformat(str, i, 4))
end

push_textdomain("world")

include "scripting/mapobjects.lua"

print("┏━ Running Lua for world:")
print_loading_message("┗━ took", function()
   print_loading_message("┃    Resources", function()
      set_loading_message(_("Resources"), 1)
      include "world/resources/init.lua"
   end)

   print_loading_message("┃    Terrains", function()
      set_loading_message(_("Terrains"), 2)

-- RST
-- .. function:: new_editor_terrain_category{table}
--
--    This function adds the definition for a category in the "Terrains" tool.
--    Only terrains can be in this editor category. The parameter `table` is described below.

      world:new_editor_terrain_category{
         name = "summer",
         descname = _ "Summer",
         picture = "world/pics/editor_terrain_category_green.png",
         items_per_row = 6,
      }
      world:new_editor_terrain_category{
         name = "wasteland",
         descname = _ "Wasteland",
         picture = "world/pics/editor_terrain_category_wasteland.png",
         items_per_row = 6,
      }
      world:new_editor_terrain_category{
         name = "winter",
         descname = _ "Winter",
         picture = "world/pics/editor_terrain_category_winter.png",
         items_per_row = 6,
      }
      world:new_editor_terrain_category{
         name = "desert",
         descname = _ "Desert",
         picture = "world/pics/editor_terrain_category_desert.png",
         items_per_row = 6,
      }

      include "world/terrains/init.lua"
   end)

   print_loading_message("┃    Immovables", function()
      set_loading_message(_("Immovables"), 3)
-- RST
-- .. function:: new_editor_immovable_category{table}
--
--    This function adds the definition for a category in the "Immovables" tool.
--    Only immovables can be in this editor category. The parameter `table` is described below.

      world:new_editor_immovable_category{
         name = "miscellaneous",
         descname = _ "Miscellaneous",
         picture = "world/immovables/ruin5/idle.png",
         items_per_row = 6,
      }

      include "world/immovables/pebble1/init.lua"
      include "world/immovables/pebble2/init.lua"
      include "world/immovables/pebble3/init.lua"
      include "world/immovables/pebble4/init.lua"
      include "world/immovables/pebble5/init.lua"
      include "world/immovables/pebble6/init.lua"
      include "world/immovables/mushroom1/init.lua"
      include "world/immovables/mushroom2/init.lua"
      include "world/immovables/manmade/snowman/init.lua"
      include "world/immovables/ruin1/init.lua"
      include "world/immovables/ruin2/init.lua"
      include "world/immovables/track_winter/init.lua"
      include "world/immovables/ruin3/init.lua"
      include "world/immovables/ruin4/init.lua"
      include "world/immovables/ruin5/init.lua"
      include "world/immovables/manmade/debris00/init.lua"
      include "world/immovables/manmade/debris02/init.lua"
      include "world/immovables/manmade/debris01/init.lua"
      include "world/immovables/manmade/bar-ruin00/init.lua"
      include "world/immovables/manmade/bar-ruin02/init.lua"
      include "world/immovables/manmade/bar-ruin03/init.lua"
      include "world/immovables/manmade/bar-ruin01/init.lua"
      include "world/immovables/skeleton1/init.lua"
      include "world/immovables/skeleton3/init.lua"
      include "world/immovables/skeleton2/init.lua"
      include "world/immovables/skeleton4/init.lua"

      -- Artifacts
      world:new_editor_immovable_category{
         name = "artifacts",
         descname = _ "Artifacts" .. "<br>" .. _ "These immovables are used by the win condition “Artifacts”.",
         picture = "world/immovables/manmade/artifacts/artifact00/idle.png",
         items_per_row = 6,
      }

      include "world/immovables/manmade/artifacts/artifact00/init.lua"
      include "world/immovables/manmade/artifacts/artifact01/init.lua"
      include "world/immovables/manmade/artifacts/artifact02/init.lua"
      include "world/immovables/manmade/artifacts/artifact03/init.lua"

      -- Plants
      world:new_editor_immovable_category{
         name = "plants",
         descname = _ "Plants",
         picture = "world/immovables/cactus3/idle.png",
         items_per_row = 8,
      }

      include "world/immovables/grass1/init.lua"
      include "world/immovables/grass2/init.lua"
      include "world/immovables/grass3/init.lua"
      include "world/immovables/bush1/init.lua"
      include "world/immovables/bush2/init.lua"
      include "world/immovables/bush3/init.lua"
      include "world/immovables/bush4/init.lua"
      include "world/immovables/bush5/init.lua"
      include "world/immovables/cactus1/init.lua"
      include "world/immovables/cactus3/init.lua"
      include "world/immovables/cactus4/init.lua"
      include "world/immovables/cactus2/init.lua"

      -- Standing Stones
      world:new_editor_immovable_category{
         name = "standing_stones",
         descname = _ "Standing Stones",
         picture = "world/immovables/standing_stones/standing_stone4_desert/idle.png",
         items_per_row = 4,
      }

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
      world:new_editor_immovable_category{
         name = "rocks",
         descname = _ "Rocks",
         picture = "world/immovables/rocks/greenland_rocks6/idle.png",
         items_per_row = 6,
      }

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
      world:new_editor_immovable_category{
         name = "trees_dead",
         descname = _ "Dead Trees",
         picture = "world/immovables/trees/deadtree2/idle.png",
         items_per_row = 8,
      }

      world:new_editor_immovable_category{
         name = "trees_coniferous",
         descname = _ "Coniferous Trees",
         picture = "world/immovables/trees/spruce/menu.png",
         items_per_row = 8,
      }

      world:new_editor_immovable_category{
         name = "trees_deciduous",
         descname = _ "Deciduous Trees",
         picture = "world/immovables/trees/alder/menu.png",
         items_per_row = 8,
      }

      world:new_editor_immovable_category{
         name = "trees_palm",
         descname = _ "Palm Trees",
         picture = "world/immovables/trees/palm_borassus/menu.png",
         items_per_row = 8,
      }

      world:new_editor_immovable_category{
         name = "trees_wasteland",
         descname = _ "Wasteland Trees",
         picture = "world/immovables/trees/umbrella_red/menu.png",
         items_per_row = 8,
      }

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
   end)

   print_loading_message("┃    Critters", function()
      set_loading_message(_("Animals"), 4)

-- RST
-- .. function:: new_editor_critter_category{table}
--
--    This function adds the definition for a category in the "Animals" tool.
--    Only critters can be in this editor category.

      world:new_editor_critter_category {
         name = "critters_herbivores",
         -- TRANSLATORS: A category in the editor for placing animals on the map.
         descname = _ "Herbivores",
         picture = "world/critters/sheep/idle_00.png",
         items_per_row = 10,
      }

      include "world/critters/bunny/init.lua"
      include "world/critters/sheep/init.lua"
      include "world/critters/wisent/init.lua"
      include "world/critters/wildboar/init.lua"
      include "world/critters/chamois/init.lua"
      include "world/critters/deer/init.lua"
      include "world/critters/reindeer/init.lua"
      include "world/critters/stag/init.lua"
      include "world/critters/moose/init.lua"

      -- Carnivores
      world:new_editor_critter_category {
         name = "critters_carnivores",
         -- TRANSLATORS: A category in the editor for placing animals on the map.
         descname = _ "Carnivores",
         picture = "world/critters/fox/idle_00.png",
         items_per_row = 10,
      }

      include "world/critters/marten/init.lua"
      include "world/critters/badger/init.lua"
      include "world/critters/lynx/init.lua"
      include "world/critters/fox/init.lua"
      include "world/critters/wolf/init.lua"
      include "world/critters/brownbear/init.lua"

      -- Aquatic animals
      world:new_editor_critter_category {
         name = "critters_aquatic",
         -- TRANSLATORS: A category in the editor for placing animals on the map.
         descname = _ "Aquatic",
         picture = "world/critters/duck/idle_00.png",
         items_per_row = 10,
      }

      include "world/critters/duck/init.lua"
   end)
end)

-- RST
--    :arg table: This table contains all the data that the game engine will
--       add to these editor categories.
--
--    **name**
--        *Mandatory*. A string containing the internal name of this editor category
--        for reference, e.g.::
--
--            name = "summer",
--
--    **descname**
--        *Mandatory*. The translatable display name, e.g.::
--
--            descname = _"Summer",
--
--    **picture**
--        *Mandatory*. An image to represent this category in the editor tool's tab, e.g.::
--
--            picture = "world/pics/editor_terrain_category_green.png",
--
--    **items_per_row**
--        *Mandatory*. How many items will be displayed in each row by the tool, e.g.::
--
--            items_per_row = 6,

pop_textdomain()
