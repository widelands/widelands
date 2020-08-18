tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes")

tribes:new_tribe {
   name = "frisians",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {8, 26} },
      bridge_normal_e = { hotspot = {-2, 12} },
      bridge_busy_e = { hotspot = {-2, 12} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 3} },
      bridge_busy_sw = { hotspot = {36, 3} }
   },
   spritesheets = {
      flag = {
         hotspot = {11, 41},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },

   bridge_height = 8,

   -- Image file paths for this tribe's road and waterway textures
   roads = {
      busy = {
         image_dirname .. "roadt_busy.png",
      },
      normal = {
         image_dirname .. "roadt_normal_00.png",
         image_dirname .. "roadt_normal_01.png",
      },
      waterway = {
         image_dirname .. "waterway_0.png",
      },
   },

   resource_indicators = {
      [""] = {
         [0] = "frisians_resi_none",
      },
      coal = {
         [10] = "frisians_resi_coal_1",
         [20] = "frisians_resi_coal_2",
      },
      iron = {
         [10] = "frisians_resi_iron_1",
         [20] = "frisians_resi_iron_2",
      },
      gold = {
         [10] = "frisians_resi_gold_1",
         [20] = "frisians_resi_gold_2",
      },
      stones = {
         [10] = "frisians_resi_stones_1",
         [20] = "frisians_resi_stones_2",
      },
      water = {
         [100] = "frisians_resi_water",
      },
   },

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
      {
         -- Building Materials
         {
            name = "log",
            preciousness = 4
         },
         {
            name = "granite",
            default_target_quantity = 30,
            preciousness = 3
         },
         {
            name = "clay",
            default_target_quantity = 30,
            preciousness = 9
         },
         {
            name = "brick",
            default_target_quantity = 40,
            preciousness = 3
         },
         {
            name = "reed",
            preciousness = 8
         },
         {
            name = "fur",
            default_target_quantity = 10,
            preciousness = 1
         },
         {
            name = "cloth",
            default_target_quantity = 10,
            preciousness = 0
         },
      },
      {
         -- Food
         {
            name = "fruit",
            preciousness = 1
         },
         {
            name = "water",
            preciousness = 2
         },
         {
            name = "barley",
            preciousness = 25
         },
         {
            name = "honey",
            preciousness = 1
         },
         {
            name = "bread_frisians",
            default_target_quantity = 20,
            preciousness = 3
         },
         {
            name = "honey_bread",
            default_target_quantity = 20,
            preciousness = 5
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness = 3
         },
         {
            name = "mead",
            default_target_quantity = 15,
            preciousness = 5
         },
         {
            name = "fish",
            default_target_quantity = 20,
            preciousness = 1
         },
         {
            name = "meat",
            preciousness = 2
         },
         {
            name = "smoked_fish",
            default_target_quantity = 20,
            preciousness = 5
         },
         {
            name = "smoked_meat",
            default_target_quantity = 10,
            preciousness = 7
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 3
         },
         {
            name = "meal",
            default_target_quantity = 5,
            preciousness = 6
         }
      },
      {
         -- Mining
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 40
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 2
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 3
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 6
         },
         {
            name = "scrap_iron",
            preciousness = 0
         },
         {
            name = "scrap_metal_mixed",
            preciousness = 1
         },
         {
            name = "fur_garment_old",
            preciousness = 0
         }
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 3,
            preciousness = 0
         },
         {
            name = "felling_ax",
            default_target_quantity = 3,
            preciousness = 0
         },
         {
            name = "shovel",
            default_target_quantity = 4,
            preciousness = 0
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 0
         },
         {
            name = "fishing_net",
            default_target_quantity = 2,
            preciousness = 0
         },
         {
            name = "hunting_spear",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "scythe",
            default_target_quantity = 2,
            preciousness = 0
         },
         {
            name = "bread_paddle",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "fire_tongs",
            default_target_quantity = 2,
            preciousness = 0
         },
         {
            name = "basket",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "needles",
            default_target_quantity = 1,
            preciousness = 0
         }
      },
      {
         -- Weapons & Armor
         {
            name = "sword_short",
            default_target_quantity = 30,
            preciousness = 4
         },
         {
            name = "sword_long",
            default_target_quantity = 2,
            preciousness = 3
         },
         {
            name = "sword_broad",
            default_target_quantity = 2,
            preciousness = 3
         },
         {
            name = "sword_double",
            default_target_quantity = 2,
            preciousness = 3
         },
         {
            name = "fur_garment",
            default_target_quantity = 30,
            preciousness = 3
         },
         {
            name = "fur_garment_studded",
            default_target_quantity = 2,
            preciousness = 3,
         },
         {
            name = "fur_garment_golden",
            default_target_quantity = 2,
            preciousness = 3
         },
         {
            name = "helmet",
            default_target_quantity = 2,
            preciousness = 3
         },
         {
            name = "helmet_golden",
            default_target_quantity = 2,
            preciousness = 3
         }
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         { name = "frisians_carrier" },
         { name = "frisians_ferry" },
         {
            name = "frisians_reindeer",
            default_target_quantity = 10,
            preciousness = 2
         },
         { name = "frisians_reindeer_breeder" }
      },
      {
         -- Building Materials
         { name = "frisians_stonemason" },
         { name = "frisians_woodcutter" },
         { name = "frisians_forester" },
         { name = "frisians_claydigger" },
         { name = "frisians_brickmaker" },
         { name = "frisians_builder" },
         { name = "frisians_reed_farmer" },
         { name = "frisians_seamstress" },
         { name = "frisians_seamstress_master" },
         { name = "frisians_shipwright" }
      },
      {
         -- Food
         { name = "frisians_fisher" },
         { name = "frisians_hunter" },
         { name = "frisians_farmer" },
         { name = "frisians_berry_farmer" },
         { name = "frisians_fruit_collector" },
         { name = "frisians_smoker" },
         { name = "frisians_beekeeper" },
         { name = "frisians_baker" },
         { name = "frisians_baker_master" },
         { name = "frisians_brewer" },
         { name = "frisians_brewer_master" },
         { name = "frisians_landlady" }
      },
      {
         -- Mining
         { name = "frisians_geologist" },
         { name = "frisians_miner" },
         { name = "frisians_miner_master" },
         { name = "frisians_charcoal_burner" },
         { name = "frisians_smelter" },
         { name = "frisians_blacksmith" },
         { name = "frisians_blacksmith_master" }
      },
      {
         -- Military
         {
            name = "frisians_soldier",
            default_target_quantity = 10,
            preciousness = 5
         },
         { name = "frisians_trainer" },
         { name = "frisians_scout" }
      }
   },

   immovables = {
      "ashes",
      "destroyed_building",
      "berry_bush_blueberry_tiny",
      "berry_bush_blueberry_small",
      "berry_bush_blueberry_medium",
      "berry_bush_blueberry_ripe",
      "berry_bush_currant_red_tiny",
      "berry_bush_currant_red_small",
      "berry_bush_currant_red_medium",
      "berry_bush_currant_red_ripe",
      "berry_bush_juniper_tiny",
      "berry_bush_juniper_small",
      "berry_bush_juniper_medium",
      "berry_bush_juniper_ripe",
      "berry_bush_raspberry_tiny",
      "berry_bush_raspberry_small",
      "berry_bush_raspberry_medium",
      "berry_bush_raspberry_ripe",
      "berry_bush_currant_black_tiny",
      "berry_bush_currant_black_small",
      "berry_bush_currant_black_medium",
      "berry_bush_currant_black_ripe",
      "berry_bush_strawberry_tiny",
      "berry_bush_strawberry_small",
      "berry_bush_strawberry_medium",
      "berry_bush_strawberry_ripe",
      "berry_bush_stink_tree_tiny",
      "berry_bush_stink_tree_small",
      "berry_bush_stink_tree_medium",
      "berry_bush_stink_tree_ripe",
      "berry_bush_desert_hackberry_tiny",
      "berry_bush_desert_hackberry_small",
      "berry_bush_desert_hackberry_medium",
      "berry_bush_desert_hackberry_ripe",
      "berry_bush_sea_buckthorn_tiny",
      "berry_bush_sea_buckthorn_small",
      "berry_bush_sea_buckthorn_medium",
      "berry_bush_sea_buckthorn_ripe",
      "barleyfield_tiny",
      "barleyfield_small",
      "barleyfield_medium",
      "barleyfield_ripe",
      "barleyfield_harvested",
      "pond_dry",
      "pond_growing",
      "pond_mature",
      "pond_burning",
      "pond_coal",
      "reedfield_tiny",
      "reedfield_small",
      "reedfield_medium",
      "reedfield_ripe",
      "frisians_resi_none",
      "frisians_resi_water",
      "frisians_resi_coal_1",
      "frisians_resi_iron_1",
      "frisians_resi_gold_1",
      "frisians_resi_stones_1",
      "frisians_resi_coal_2",
      "frisians_resi_iron_2",
      "frisians_resi_gold_2",
      "frisians_resi_stones_2",
      "frisians_shipconstruction",
      -- These non-frisian immovables can be used by bee-keepers
      "wheatfield_medium",
      "cornfield_medium",
      "blackrootfield_medium",
      "grapevine_medium",
      -- Used by the fruit collector
      "grapevine_ripe",
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      "frisians_headquarters",
      "frisians_warehouse",
      "frisians_port",

      -- Small
      "frisians_quarry",
      "frisians_woodcutters_house",
      "frisians_foresters_house",
      "frisians_hunters_house",
      "frisians_fishers_house",
      "frisians_reed_farm",
      "frisians_well",
      "frisians_clay_pit",
      "frisians_charcoal_burners_house",
      "frisians_berry_farm",
      "frisians_collectors_house",
      "frisians_beekeepers_house",
      "frisians_aqua_farm",
      "frisians_scouts_house",

      -- Medium
      "frisians_brick_kiln",
      "frisians_furnace",
      "frisians_recycling_center",
      "frisians_blacksmithy",
      "frisians_armor_smithy_small",
      "frisians_armor_smithy_large",
      "frisians_sewing_room",
      "frisians_tailors_shop",
      "frisians_charcoal_kiln",
      "frisians_smokery",
      "frisians_bakery",
      "frisians_honey_bread_bakery",
      "frisians_brewery",
      "frisians_mead_brewery",
      "frisians_tavern",
      "frisians_drinking_hall",
      "frisians_barracks",

      -- Big
      "frisians_reindeer_farm",
      "frisians_farm",

      -- Mines
      "frisians_rockmine",
      "frisians_rockmine_deep",
      "frisians_coalmine",
      "frisians_coalmine_deep",
      "frisians_ironmine",
      "frisians_ironmine_deep",
      "frisians_goldmine",
      "frisians_goldmine_deep",

      -- Training Sites
      "frisians_training_camp",
      "frisians_training_arena",

      -- Military Sites
      "frisians_wooden_tower",
      "frisians_wooden_tower_high",
      "frisians_sentinel",
      "frisians_outpost",
      "frisians_tower",
      "frisians_fortress",

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      "frisians_ferry_yard",
      "frisians_shipyard",
      "frisians_weaving_mill",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Amrum"),
      -- TRANSLATORS: This frisian ship is named after a no longer existing island in Northern Frisia
      pgettext("shipname", "Alt-Nordstrand"),
      -- TRANSLATORS: This frisian ship is named after a peninsula in Northern Frisia
      pgettext("shipname", "Eiderstedt"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Föhr"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Gröde"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Habel"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Hamburger Hallig"),
      -- TRANSLATORS: This frisian ship is named after an island in the North Sea
      pgettext("shipname", "Helgoland"),
      -- TRANSLATORS: This frisian ship is named after a region in Northern Frisia
      pgettext("shipname", "Hever"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Hooge"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Japsand"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Kormoransand"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Langeneß"),
      -- TRANSLATORS: This frisian ship is named after a water body in Northern Frisia
      pgettext("shipname", "Norderaue"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Norderoog"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Norderoogsand"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Nordstrand"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Nordstrandischmoor"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Oland"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Pellworm"),
      -- TRANSLATORS: This frisian ship is named after a no longer existing island in Northern Frisia
      pgettext("shipname", "Strand"),
      -- TRANSLATORS: This frisian ship is named after a water body in Northern Frisia
      pgettext("shipname", "Süderaue"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Süderoog"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Süderoogsand"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Südfall"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Sylt"),
      -- TRANSLATORS: This frisian ship is named after a region in Northern Frisia
      pgettext("shipname", "Uthlande"),
   },

   -- Special types
   builder = "frisians_builder",
   carrier = "frisians_carrier",
   carrier2 = "frisians_reindeer",
   geologist = "frisians_geologist",
   soldier = "frisians_soldier",
   ship = "frisians_ship",
   ferry = "frisians_ferry",
   port = "frisians_port",
}

pop_textdomain()
