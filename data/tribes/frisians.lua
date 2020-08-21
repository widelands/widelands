tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/frisians/"

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
         "tribes/images/frisians/waterway_0.png",
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
         "log",
         "granite",
         "clay",
         "brick",
         "reed",
         "fur",
         "cloth"
      },
      {
         -- Food
          "fruit",
          "water",
          "barley",
          "honey",
          "bread_frisians",
          "honey_bread",
          "beer",
          "mead",
          "fish",
          "meat",
          "smoked_fish",
          "smoked_meat",
          "ration",
          "meal"
      },
      {
         -- Mining
          "coal",
          "iron_ore",
          "iron",
          "gold_ore",
          "gold",
          "scrap_iron",
          "scrap_metal_mixed",
          "fur_garment_old",
      },
      {
         -- Tools
          "pick",
          "felling_ax",
          "shovel",
          "hammer",
          "fishing_net",
          "hunting_spear",
          "scythe",
          "bread_paddle",
          "kitchen_tools",
          "fire_tongs",
          "basket",
          "needles"
      },
      {
         -- Weapons & Armor
          "sword_short",
          "sword_long",
          "sword_broad",
          "sword_double",
          "fur_garment",
          "fur_garment_studded",
          "fur_garment_golden",
          "helmet",
          "helmet_golden",
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         "frisians_carrier",
         "frisians_ferry",
         "frisians_reindeer",
         "frisians_reindeer_breeder"
      },
      {
         -- Building Materials
         "frisians_stonemason",
         "frisians_woodcutter",
         "frisians_forester",
         "frisians_claydigger",
         "frisians_brickmaker",
         "frisians_builder",
         "frisians_reed_farmer",
         "frisians_seamstress",
         "frisians_seamstress_master",
         "frisians_shipwright"
      },
      {
         -- Food
         "frisians_fisher",
         "frisians_hunter",
         "frisians_farmer",
         "frisians_berry_farmer",
         "frisians_fruit_collector",
         "frisians_smoker",
         "frisians_beekeeper",
         "frisians_baker",
         "frisians_baker_master",
         "frisians_brewer",
         "frisians_brewer_master",
         "frisians_landlady"
      },
      {
         -- Mining
         "frisians_geologist",
         "frisians_miner",
         "frisians_miner_master",
         "frisians_charcoal_burner",
         "frisians_smelter",
         "frisians_blacksmith",
         "frisians_blacksmith_master"
      },
      {
         -- Military
         "frisians_soldier",
         "frisians_trainer",
         "frisians_scout"
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
