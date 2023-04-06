-- This file is only used by the random map generator. It is not part of the
-- world definition (i.e. it is not sourced by world/init.lua) and is only
-- loaded when the random map generator is initialized in the Editor.


------------------------
--  Former greenland  --
------------------------

greenland = {
   heights = {
      ocean = 10,
      shelf = 10,
      shallow = 10,
      coast = 11,
      upperland = 14,
      mountainfoot = 15,
      mountain = 16,
      snow = 39,
      summit = 45,
   },
   areas = {
      water = {
         {
            name = "ocean",
            weight = 1,
            ocean_terrains = { "summer_water" },
            shelf_terrains = { "summer_water" },
            shallow_terrains = { "summer_water" },
         }
      },
      land = {
         {
            name = "beach",
            weight = 2,
            coast_terrains = { "summer_beach" },
            land_terrains = { "summer_meadow2", "summer_meadow3" },
            upper_terrains = { "summer_meadow2", "summer_meadow3" },
         },
         {
            name = "meadow",
            weight = 1,
            coast_terrains = { "summer_beach" },
            land_terrains = { "summer_meadow1" },
            upper_terrains = { "summer_meadow2", "summer_meadow3" },
         }
      },
      wasteland = {
         {
            name = "swamp",
            weight = 1,
            inner_terrains = { "summer_swamp" },
            outer_terrains = { "summer_swamp" },
         },
         {
            name = "tundra",
            inner_terrains = { "summer_steppe_barren" },
            weight = 1,
            outer_terrains = { "summer_steppe" },
         }
      },
      mountains = {
         {
            name = "mountains",
            weight = 1,
            mountainfoot_terrains = { "summer_mountain_meadow" },
            mountain_terrains = { "summer_mountain1", "summer_mountain2", "summer_mountain3", "summer_mountain4" },
            snow_terrains = { "summer_snow" },
         }
      }
   }, -- areas
   bob_categories = {
      {
         name = "trees",
         immovables = {
            "aspen_summer_old", "oak_summer_old", "spruce_summer_old",
            "alder_summer_old", "birch_summer_old", "beech_summer_old",
            "larch_summer_old", "rowan_summer_old"
         },
         critters = { "brownbear", "chamois", "moose", "lynx", "reindeer", "stag", "wildboar", "wolf" },
      },
      {
         name = "bushes",
         immovables = { "bush1", "bush2", "bush3", "bush4", "bush5" },
         critters = { "bunny", "moose", "fox", "lynx", "marten", "sheep", "sheep", "wildboar", "wisent", "wolf", "wolf" },
      },
      {
         name = "grasses",
         immovables = { "grass1", "grass2", "grass3" },
         critters = { "bunny", "moose", "fox", "lynx", "marten", "sheep", "sheep", "wolf" },
      },
      {
         name = "standing_stones",
         immovables = {
            "standing_stone1_summer", "standing_stone2_summer",
            "standing_stone3_summer", "standing_stone4_summer",
            "standing_stone5_summer", "standing_stone6", "standing_stone7"
         },
         critters = {},
      },
      {
         name = "rocks",
         immovables = {
            "greenland_rocks1", "greenland_rocks2", "greenland_rocks3",
            "greenland_rocks4", "greenland_rocks5", "greenland_rocks5",
            "greenland_rocks6"
         },
         critters = {},
      },
      {
         name = "wasteland_doodads",
         immovables = {
            "skeleton1", "skeleton2", "skeleton3", "cactus1",
            "cactus2", "fallentree", "deadtree1", "deadtree2",
            "deadtree3", "deadtree4"
         },
         critters = {},
      }
   }, -- bob_categories
   land_resources = {
      {
         name = "forest",
         weight = 6,
         immovable_density = 100,
         critter_density = 20,
         land_coast_bobs = "bushes",
         land_inner_bobs = "trees",
         land_upper_bobs = "bushes",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "bushes",
      },
      {
         name = "bushland",
         weight = 2,
         immovable_density = 50,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "bushes",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "meadow",
         weight = 2,
         immovable_density = 10,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "grasses",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "rock",
         weight = 4,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "rocks",
         land_upper_bobs = "",
         wasteland_inner_bobs = "rocks",
         wasteland_outer_bobs = "rocks",
      },
      {
         name = "standing_stones",
         weight = 2,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "standing_stones",
         land_upper_bobs = "",
         wasteland_inner_bobs = "standing_stones",
         wasteland_outer_bobs = "standing_stones",
      }
   } -- land resources
}

------------------------
--  Former blackland  --
------------------------

blackland = {
   heights = {
      ocean = 10,
      shelf = 10,
      shallow = 10,
      coast = 11,
      upperland = 14,
      mountainfoot = 15,
      mountain = 16,
      snow = 39,
      summit = 45,
   },
   areas = {
      water = {
         {
            name = "ocean",
            weight = 1,
            ocean_terrains = { "wasteland_water" },
            shelf_terrains = { "wasteland_water" },
            shallow_terrains = { "wasteland_water" },
         }
      },
      land = {
         {
            name = "hardground1",
            weight = 2,
            coast_terrains = { "wasteland_beach" },
            land_terrains = { "hardground1", "hardground2" },
            upper_terrains = { "hardground1", "hardground2" },
         },
         {
            name = "hardground2",
            weight = 2,
            coast_terrains = { "wasteland_beach" },
            land_terrains = { "hardground3", "hardground4" },
            upper_terrains = { "hardground3", "hardground4" },
         },
         {
            name = "ashes",
            weight = 1,
            coast_terrains = { "ashes2" },
            land_terrains = { "ashes1" },
            upper_terrains = { "ashes1" },
         },
      },
      wasteland = {
         {
            name = "wasteland1",
            weight = 1,
            inner_terrains = { "lava", "lava-stone1" },
            outer_terrains = { "hardlava", "lava-stone2" },
         },
         {
            name = "wasteland2",
            weight = 2,
            inner_terrains = { "hardlava", "lava-stone2" },
            outer_terrains = { "hardlava", "lava-stone2" },
         },
      },
      mountains = {
         {
            name = "mountains",
            weight = 1,
            mountainfoot_terrains = { "hardlava" },
            mountain_terrains = {
               "wasteland_mountain1", "wasteland_mountain2",
               "wasteland_mountain3", "wasteland_mountain4"
            },
            snow_terrains = { "lava", "lava-stone1", "lava-stone2" },
         }
      }
   }, -- areas
   bob_categories = {
      {
         name = "trees",
         immovables = {
            "umbrella_red_wasteland_old",
            "mushroom_dark_wasteland_mature",
            "twine_wasteland_old", "umbrella_green_wasteland_old",
            "mushroom_red_wasteland_old",
            "mushroom_green_wasteland_old", "cirrus_wasteland_old",
            "liana_wasteland_old"
         },
         critters = { "deer", "reindeer", "wildboar" },
      },
      {
         name = "bushes",
         immovables = { "bush1", "bush2", "bush3", "bush4", "bush5" },
         critters = { "bunny", "fox", "sheep", "wildboar" },
      },
      {
         name = "grasses",
         immovables = { "grass1", "grass2", "grass3" },
         critters = { "bunny", "fox", "sheep" },
      },
      {
         name = "standing_stones",
         immovables = {
            "standing_stone1_wasteland",
            "standing_stone2_wasteland",
            "standing_stone3_wasteland",
            "standing_stone4_wasteland",
            "standing_stone5_wasteland",
         },
         critters = {},
      },
      {
         name = "rocks",
         immovables = {
            "blackland_rocks1", "blackland_rocks2",
            "blackland_rocks3", "blackland_rocks4",
            "blackland_rocks5", "blackland_rocks6",
         },
         critters = {},
      },
      {
         name = "wasteland_doodads",
         immovables = {
            "skeleton1", "skeleton2", "skeleton3", "cactus1",
            "cactus2", "fallentree", "deadtree1", "deadtree2",
            "deadtree3", "deadtree4"
         },
         critters = {},
      }
   }, -- bob_categories
   land_resources = {
      {
         name = "forest",
         weight = 6,
         immovable_density = 40,
         critter_density = 20,
         land_coast_bobs = "bushes",
         land_inner_bobs = "trees",
         land_upper_bobs = "bushes",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "bushes",
      },
      {
         name = "bushland",
         weight = 2,
         immovable_density = 50,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "bushes",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "meadow",
         weight = 2,
         immovable_density = 10,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "grasses",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "rock",
         weight = 4,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "rocks",
         land_upper_bobs = "",
         wasteland_inner_bobs = "rocks",
         wasteland_outer_bobs = "rocks",
      },
      {
         name = "standing_stones",
         weight = 2,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "standing_stones",
         land_upper_bobs = "",
         wasteland_inner_bobs = "standing_stones",
         wasteland_outer_bobs = "standing_stones",
      }
   } -- land resources
}

-- ---------------------
-- --  Former desert  --
-- ---------------------

desert = {
   heights = {
      ocean = 10,
      shelf = 10,
      shallow = 10,
      coast = 11,
      upperland = 14,
      mountainfoot = 15,
      mountain = 16,
      snow = 39,
      summit = 45,
   },
   areas = {
      water = {
         {
            name = "water",
            weight = 1,
            ocean_terrains = { "desert_water" },
            shelf_terrains = { "desert_water" },
            shallow_terrains = { "desert_water" },
         }
      },
      land = {
         {
            name = "meadow_land",
            weight = 1,
            coast_terrains = { "desert_beach" },
            land_terrains = { "meadow" },
            upper_terrains = { "mountainmeadow" },
         },
      },
      wasteland = {
         {
            name = "desert",
            weight = 1,
            inner_terrains = { "desert1", "desert2", "desert3", "desert4" },
            outer_terrains = { "desert1", "desert2", "desert3", "desert4" },
         },
      },
      mountains = {
         {
            name = "mountains",
            weight = 1,
            mountainfoot_terrains = { "mountainmeadow" },
            mountain_terrains = {
               "mountain1", "mountain2"
            },
            snow_terrains = {
               "mountain3", "mountain4"
            },
         }
      }
   }, -- areas
   bob_categories = {
      {
         name = "trees",
         immovables = {
            "palm_date_desert_old", "palm_borassus_desert_old",
            "palm_coconut_desert_old", "palm_roystonea_desert_old",
            "palm_oil_desert_old", "beech_summer_old",
            "larch_summer_old", "rowan_summer_old",
         },
         critters = { "deer" },
      },
      {
         name = "bushes",
         immovables = { "bush1", "bush4", "bush5" },
         critters = { "bunny", "fox", "sheep" },
      },
      {
         name = "grasses",
         immovables = {
            "grass1", "grass2", "grass3", "ruin1", "ruin2",
            "ruin3", "ruin4",
         },
         critters = { "bunny", "fox", "sheep" },
      },
      {
         name = "standing_stones",
         immovables = {
            "standing_stone1_desert",
            "standing_stone2_desert",
            "standing_stone3_desert",
            "standing_stone4_desert",
            "standing_stone5_desert",
         },
         critters = {},
      },
      {
         name = "rocks",
         immovables = {
            "desert_rocks1", "desert_rocks2",
            "desert_rocks3", "desert_rocks4",
            "desert_rocks5", "desert_rocks6",
         },
         critters = {},
      },
      {
         name = "wasteland_doodads",
         immovables = {
            "skeleton1", "skeleton2", "skeleton3", "cactus1",
            "cactus2", "fallentree", "deadtree1", "deadtree5",
            "deadtree6", "deadtree4"
         },
         critters = {},
      }
   }, -- bob_categories
   land_resources = {
      {
         name = "forest",
         weight = 6,
         immovable_density = 40,
         critter_density = 20,
         land_coast_bobs = "bushes",
         land_inner_bobs = "trees",
         land_upper_bobs = "bushes",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "bushes",
      },
      {
         name = "bushland",
         weight = 2,
         immovable_density = 50,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "bushes",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "meadow",
         weight = 2,
         immovable_density = 10,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "grasses",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "rock",
         weight = 4,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "rocks",
         land_upper_bobs = "",
         wasteland_inner_bobs = "rocks",
         wasteland_outer_bobs = "rocks",
      },
      {
         name = "standing_stones",
         weight = 2,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "standing_stones",
         land_upper_bobs = "",
         wasteland_inner_bobs = "standing_stones",
         wasteland_outer_bobs = "standing_stones",
      }
   } -- land resources
}


-- -------------------------
-- --  Former winterland  --
-- -------------------------

winterland = {
   heights = {
      ocean = 10,
      shelf = 10,
      shallow = 10,
      coast = 11,
      upperland = 14,
      mountainfoot = 15,
      mountain = 16,
      snow = 39,
      summit = 45,
   },
   areas = {
      water = {
         {
            name = "water",
            weight = 1,
            ocean_terrains = { "winter_water" },
            shelf_terrains = { "winter_water" },
            shallow_terrains = { "winter_water" },
         },
         {
            name = "ice_floes",
            weight = 1,
            ocean_terrains = { "ice_floes", "ice_floes2" },
            shelf_terrains = { "ice_floes", "ice_floes2" },
            shallow_terrains = { "ice_floes", "ice_floes2" },
         },
         {
            name = "ice",
            weight = 1,
            ocean_terrains = { "ice" },
            shelf_terrains = { "ice" },
            shallow_terrains = { "ice" },
         }
      },
      land = {
         {
            name = "land",
            weight = 1,
            coast_terrains = { "winter_beach" },
            land_terrains = { "winter_taiga1", "winter_taiga2", "winter_taiga3" },
            upper_terrains = { "winter_taiga1", "winter_taiga2", "winter_taiga3" },
         },
      },
      wasteland = {
         {
            name = "wasteland",
            weight = 1,
            inner_terrains = { "tundra_taiga" },
            outer_terrains = { "winter_tundra", "snow" },
         },
      },
      mountains = {
         {
            name = "mountains",
            weight = 1,
            mountainfoot_terrains = { "tundra_taiga" },
            mountain_terrains = { "winter_mountain3", "winter_mountain4" },
            snow_terrains = { "winter_mountain1", "winter_mountain2" },
         }
      }
   }, -- areas
   bob_categories = {
      {
         name = "trees",
         immovables = {
            "aspen_summer_old", "oak_summer_old",
            "spruce_summer_old", "maple_winter_old",
            "birch_summer_old", "beech_summer_old",
            "larch_summer_old", "rowan_summer_old",
         },
         critters = { "deer", "moose", "lynx", "reindeer", "wolf" },
      },
      {
         name = "bushes",
         immovables = { "bush1", "bush2", "bush3", "bush4", "bush5" },
         critters = {
            "wisent", "wolf", "bunny", "moose", "fox", "lynx",
            "wolf", "marten", "sheep"
         },
      },
      {
         name = "grasses",
         immovables = { "grass1", "grass2", "grass3" },
         critters = {
            "wisent", "wolf", "bunny", "moose", "fox", "lynx",
            "wolf", "marten", "sheep"
         },
      },
      {
         name = "standing_stones",
         immovables = {
            "standing_stone1_winter",
            "standing_stone2_winter",
            "standing_stone3_winter",
            "standing_stone4_winter",
            "standing_stone5_winter",
         },
         critters = {},
      },
      {
         name = "rocks",
         immovables = {
            "winterland_rocks1", "winterland_rocks2",
            "winterland_rocks3", "winterland_rocks4",
            "winterland_rocks5", "winterland_rocks6",
         },
         critters = {},
      },
      {
         name = "wasteland_doodads",
         immovables = {
            "skeleton1", "skeleton2", "skeleton3", "deadtree1",
            "deadtree2", "deadtree3", "deadtree4"
         },
         critters = {},
      }
   }, -- bob_categories
   land_resources = {
      {
         name = "forest",
         weight = 6,
         immovable_density = 100,
         critter_density = 20,
         land_coast_bobs = "bushes",
         land_inner_bobs = "trees",
         land_upper_bobs = "bushes",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "bushes",
      },
      {
         name = "bushland",
         weight = 2,
         immovable_density = 50,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "bushes",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "meadow",
         weight = 2,
         immovable_density = 10,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "grasses",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "rock",
         weight = 4,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "rocks",
         land_upper_bobs = "",
         wasteland_inner_bobs = "rocks",
         wasteland_outer_bobs = "rocks",
      },
      {
         name = "standing_stones",
         weight = 2,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "standing_stones",
         land_upper_bobs = "",
         wasteland_inner_bobs = "standing_stones",
         wasteland_outer_bobs = "standing_stones",
      }
   } -- land resources
}

return {
   summer = greenland,
   wasteland = blackland,
   desert = desert,
   winter = winterland
}
