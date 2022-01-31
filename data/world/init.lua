-- RST
-- init.lua
-- --------
--
-- World initialization for the editor.
-- All world entities are loaded via this file.
-- In a game, world entities are loaded on demand using their ``register.lua`` files.
-- See :ref:`lua_world_defining_units` for more details.
--
-- This file also defines the editor categories for world elements like terrains or
-- immovables so that they will be added to their respective editor tools (Place Terrain,
-- Place Immovable etc.).
--
-- Returns a table with 4 keys:
--
-- * ``critters``: A table of editor categories for placing animals in the editor
-- * ``immovables``: A table of editor categories for placing immovables in the editor
-- * ``resources``: A list of all resources: ``{ "resource_coal", "resource_gold", ... }``
-- * ``terrains``: A table of editor categories for placing terrains in the editor
--
-- For making the editor category names translatable, we also need to push/pop the correct textdomain.
--
-- An editor category has the following table entries:
--
-- **name**
--     *Mandatory*. A string containing the internal name of this editor category
--     for reference by UI code, e.g.::
--
--         name = "summer",
--
-- **descname**
--     *Mandatory*. The translatable display name, e.g.::
--
--         descname = _("Summer"),
--
-- **picture**
--     *Mandatory*. An image to represent this category in the editor tool's tab, e.g.::
--
--         picture = "world/pics/editor_terrain_category_green.png",
--
-- **items_per_row**
--     *Mandatory*. How many items will be displayed in each row by the tool, e.g.::
--
--         items_per_row = 6,
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("world")
--
--    local result = {
--       -- Items shown in the place critter tool. Each subtable is a tab in the tool.
--       critters = {
--          {
--             name = "critters_herbivores",
--             descname = _("Herbivores"),
--             picture = "world/critters/sheep/menu.png",
--             items_per_row = 10,
--             items = {
--                "bunny",
--                "sheep",
--             }
--          },
--          {
--             name = "critters_carnivores",
--             descname = _("Carnivores"),
--             picture = "world/critters/fox/menu.png",
--             items_per_row = 10,
--             items = {
--                "marten",
--                "badger",
--                "lynx",
--                "fox",
--                "wolf",
--                "brownbear",
--             }
--          },
--          ...
--       },
--
--       -- Items shown in the place immovable tool. Each subtable is a tab in the tool.
--       immovables = {
--          {
--             name = "immovables_miscellaneous",
--             ...
--          },
--          ...
--       },
--
--       -- Items shown in the set resources tool.
--       resources = {
--          "resource_coal",
--          "resource_gold",
--          "resource_iron",
--          "resource_stones",
--          "resource_water",
--          "resource_fish",
--       },
--
--       -- Items shown in the place terrain tool. Each subtable is a tab in the tool.
--       terrains = {
--          {
--             name = "terrains_summer",
--             ...
--          },
--          ...
--       }
--    }
--    pop_textdomain()
--    return result

push_textdomain("world")

local result = {
   -- Items shown in the place critter tool. Each subtable is a tab in the tool.
   critters = {
      {
         name = "critters_herbivores",
         -- TRANSLATORS: A category in the editor for placing animals on the map.
         descname = _("Herbivores"),
         picture = "world/critters/sheep/menu.png",
         items_per_row = 10,
         items = {
            "bunny",
            "sheep",
            "wisent",
            "wildboar",
            "chamois",
            "deer",
            "reindeer",
            "stag",
            "moose",
         }
      },
      {
         name = "critters_carnivores",
         -- TRANSLATORS: A category in the editor for placing animals on the map.
         descname = _("Carnivores"),
         picture = "world/critters/fox/menu.png",
         items_per_row = 10,
         items = {
            "marten",
            "badger",
            "lynx",
            "fox",
            "wolf",
            "brownbear",
         }
      },
      {
         name = "critters_aquatic",
         -- TRANSLATORS: A category in the editor for placing animals on the map.
         descname = _("Aquatic"),
         picture = "world/critters/duck/menu.png",
         items_per_row = 10,
         items = {
            "duck",
         }
      }
   },

   -- Items shown in the place immovable tool. Each subtable is a tab in the tool.
   immovables = {
      {
         name = "immovables_miscellaneous",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Miscellaneous"),
         picture = "world/immovables/miscellaneous/ruin5/idle.png",
         items_per_row = 6,
         items = {
            "pebble1",
            "pebble2",
            "pebble3",
            "pebble4",
            "pebble5",
            "pebble6",

            "mushroom1",
            "mushroom2",
            "snowman",
            "ruin1",
            "ruin2",
            "track_winter",

            "ruin3",
            "ruin4",
            "ruin5",
            "debris00",
            "debris02",
            "debris01",

            "bar-ruin00",
            "bar-ruin02",
            "bar-ruin03",
            "bar-ruin01",
            "skeleton1",
            "skeleton3",

            "skeleton2",
            "skeleton4",
         }
      },
      {
         name = "immovables_artifacts",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Artifacts") .. "<br>" .. _("These immovables are used by the win condition “Artifacts”."),
         picture = "world/immovables/artifacts/artifact00/idle.png",
         items_per_row = 6,
         items = {
            "artifact00",
            "artifact01",
            "artifact02",
            "artifact03",
         }
      },
      {
         name = "immovables_plants",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Plants"),
         picture = "world/immovables/plants/cactus3/idle.png",
         items_per_row = 8,
         items = {
            "grass1",
            "grass2",
            "grass3",
            "bush1",
            "bush2",
            "bush3",
            "bush4",
            "bush5",

            "cactus1",
            "cactus3",
            "cactus4",
            "cactus2",
         }
      },
      {
         name = "immovables_standing_stones",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Standing Stones"),
         picture = "world/immovables/standing_stones/standing_stone4_desert/idle.png",
         items_per_row = 4,
         items = {
            "standing_stone1_desert",
            "standing_stone1_summer",
            "standing_stone1_wasteland",
            "standing_stone1_winter",

            "standing_stone2_desert",
            "standing_stone2_summer",
            "standing_stone2_wasteland",
            "standing_stone2_winter",

            "standing_stone3_desert",
            "standing_stone3_summer",
            "standing_stone3_wasteland",
            "standing_stone3_winter",

            "standing_stone4_desert",
            "standing_stone4_summer",
            "standing_stone4_wasteland",
            "standing_stone4_winter",

            "standing_stone5_desert",
            "standing_stone5_summer",
            "standing_stone5_wasteland",
            "standing_stone5_winter",

            "standing_stone6",
            "standing_stone7",
         }
      },
      {
         name = "immovables_rocks",
         descname = _("Rocks"),
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         picture = "world/immovables/rocks/greenland/6/rocks6.png",
         items_per_row = 6,
         items = {
            "blackland_rocks1",
            "blackland_rocks2",
            "blackland_rocks3",
            "blackland_rocks4",
            "blackland_rocks5",
            "blackland_rocks6",

            "desert_rocks1",
            "desert_rocks2",
            "desert_rocks3",
            "desert_rocks4",
            "desert_rocks5",
            "desert_rocks6",

            "greenland_rocks1",
            "greenland_rocks2",
            "greenland_rocks3",
            "greenland_rocks4",
            "greenland_rocks5",
            "greenland_rocks6",

            "winterland_rocks1",
            "winterland_rocks2",
            "winterland_rocks3",
            "winterland_rocks4",
            "winterland_rocks5",
            "winterland_rocks6",
         }
      },
      {
         name = "immovables_trees_dead",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Dead Trees"),
         picture = "world/immovables/trees/deadtree2/idle.png",
         items_per_row = 8,
         items = {
            "deadtree1",
            "deadtree2",
            "deadtree3",
            "deadtree4",
            "deadtree5",
            "deadtree6",
            "fallentree"
         }
      },
      {
         name = "immovables_trees_coniferous",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Coniferous Trees"),
         picture = "world/immovables/trees/spruce/menu.png",
         items_per_row = 8,
         items = {
            "larch_summer_sapling",
            "larch_summer_pole",
            "larch_summer_mature",
            "larch_summer_old",

            "spruce_summer_sapling",
            "spruce_summer_pole",
            "spruce_summer_mature",
            "spruce_summer_old",
         }
      },
      {
         name = "immovables_trees_deciduous",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Deciduous Trees"),
         picture = "world/immovables/trees/alder/menu.png",
         items_per_row = 8,
         items = {
            "alder_summer_sapling",
            "alder_summer_pole",
            "alder_summer_mature",
            "alder_summer_old",

            "aspen_summer_sapling",
            "aspen_summer_pole",
            "aspen_summer_mature",
            "aspen_summer_old",

            "beech_summer_sapling",
            "beech_summer_pole",
            "beech_summer_mature",
            "beech_summer_old",

            "birch_summer_sapling",
            "birch_summer_pole",
            "birch_summer_mature",
            "birch_summer_old",

            "maple_winter_sapling",
            "maple_winter_pole",
            "maple_winter_mature",
            "maple_winter_old",

            "oak_summer_sapling",
            "oak_summer_pole",
            "oak_summer_mature",
            "oak_summer_old",

            "rowan_summer_sapling",
            "rowan_summer_pole",
            "rowan_summer_mature",
            "rowan_summer_old",
         }
      },
      {
         name = "immovables_trees_palm",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Palm Trees"),
         picture = "world/immovables/trees/palm_borassus/menu.png",
         items_per_row = 8,
         items = {
            "palm_borassus_desert_sapling",
            "palm_borassus_desert_pole",
            "palm_borassus_desert_mature",
            "palm_borassus_desert_old",

            "palm_coconut_desert_sapling",
            "palm_coconut_desert_pole",
            "palm_coconut_desert_mature",
            "palm_coconut_desert_old",

            "palm_date_desert_sapling",
            "palm_date_desert_pole",
            "palm_date_desert_mature",
            "palm_date_desert_old",

            "palm_oil_desert_sapling",
            "palm_oil_desert_pole",
            "palm_oil_desert_mature",
            "palm_oil_desert_old",

            "palm_roystonea_desert_sapling",
            "palm_roystonea_desert_pole",
            "palm_roystonea_desert_mature",
            "palm_roystonea_desert_old",
         }
      },
      {
         name = "immovables_trees_wasteland",
         -- TRANSLATORS: A category in the editor for placing immovables on the map.
         descname = _("Wasteland Trees"),
         picture = "world/immovables/trees/umbrella_red/menu.png",
         items_per_row = 8,
         items = {
            "cirrus_wasteland_sapling",
            "cirrus_wasteland_pole",
            "cirrus_wasteland_mature",
            "cirrus_wasteland_old",

            "liana_wasteland_sapling",
            "liana_wasteland_pole",
            "liana_wasteland_mature",
            "liana_wasteland_old",

            "mushroom_dark_wasteland_sapling",
            "mushroom_dark_wasteland_pole",
            "mushroom_dark_wasteland_mature",
            "mushroom_dark_wasteland_old",

            "mushroom_green_wasteland_sapling",
            "mushroom_green_wasteland_pole",
            "mushroom_green_wasteland_mature",
            "mushroom_green_wasteland_old",

            "mushroom_red_wasteland_sapling",
            "mushroom_red_wasteland_pole",
            "mushroom_red_wasteland_mature",
            "mushroom_red_wasteland_old",

            "twine_wasteland_sapling",
            "twine_wasteland_pole",
            "twine_wasteland_mature",
            "twine_wasteland_old",

            "umbrella_green_wasteland_sapling",
            "umbrella_green_wasteland_pole",
            "umbrella_green_wasteland_mature",
            "umbrella_green_wasteland_old",

            "umbrella_red_wasteland_sapling",
            "umbrella_red_wasteland_pole",
            "umbrella_red_wasteland_mature",
            "umbrella_red_wasteland_old",
         }
      }
   },

   -- Items shown in the set resources tool.
   resources = {
      "resource_coal",
      "resource_gold",
      "resource_iron",
      "resource_stones",
      "resource_water",
      "resource_fish",
   },

   -- Items shown in the place terrain tool. Each subtable is a tab in the tool.
   terrains = {
      {
         name = "terrains_summer",
         -- TRANSLATORS: A category in the editor for placing terrains on the map.
         descname = _("Summer"),
         picture = "world/pics/editor_terrain_category_green.png",
         items_per_row = 6,
         items = {
            "summer_meadow1",
            "summer_meadow2",
            "summer_meadow3",
            "summer_meadow4",
            "summer_steppe",
            "summer_steppe_barren",

            "summer_mountain_meadow",
            "summer_forested_mountain1",
            "summer_forested_mountain2",
            "summer_mountain1",
            "summer_mountain2",
            "summer_mountain3",

            "summer_mountain4",
            "summer_beach",
            "summer_swamp",
            "summer_snow",
            "lava",
            "summer_water",
         }
      },
      {
         name = "terrains_wasteland",
         -- TRANSLATORS: A category in the editor for placing terrains on the map.
         descname = _("Wasteland"),
         picture = "world/pics/editor_terrain_category_wasteland.png",
         items_per_row = 6,
         items = {
            "ashes1",
            "ashes2",
            "hardground1",
            "hardground2",
            "hardground3",
            "hardground4",

            "hardlava",
            "wasteland_forested_mountain1",
            "wasteland_forested_mountain2",
            "wasteland_mountain1",
            "wasteland_mountain2",
            "wasteland_mountain3",

            "wasteland_mountain4",
            "wasteland_beach",
            "lava-stone1",
            "lava-stone2",
            "wasteland_water",
         }
      },
      {
         name = "terrains_winter",
         -- TRANSLATORS: A category in the editor for placing terrains on the map.
         descname = _("Winter"),
         picture = "world/pics/editor_terrain_category_winter.png",
         items_per_row = 6,
         items = {
            "winter_taiga1",
            "winter_taiga2",
            "winter_taiga3",
            "tundra_taiga",
            "winter_tundra",
            "snow",

            "winter_forested_mountain1",
            "winter_forested_mountain2",
            "winter_mountain1",
            "winter_mountain2",
            "winter_mountain3",
            "winter_mountain4",

            "ice",
            "winter_beach",
            "ice_floes",
            "ice_floes2",
            "winter_water",
         }
      },
      {
         name = "terrains_desert",
         -- TRANSLATORS: A category in the editor for placing terrains on the map.
         descname = _("Desert"),
         picture = "world/pics/editor_terrain_category_desert.png",
         items_per_row = 6,
         items = {
            "desert4",
            "drysoil",
            "desert_steppe",
            "meadow",
            "mountainmeadow",
            "highmountainmeadow",

            "desert_forested_mountain1",
            "desert_forested_mountain2",
            "mountain1",
            "mountain2",
            "mountain3",
            "mountain4",

            "desert1",
            "desert2",
            "desert3",
            "desert_beach",
            "desert_water",
         }
      },
   }
}

pop_textdomain()

return result
