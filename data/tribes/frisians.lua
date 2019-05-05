dirname = path.dirname (__file__)

tribes:new_tribe {
   name = "frisians",

   animations = {
      frontier = {
         pictures = path.list_files (dirname .. "images/frisians/frontier_??.png"),
         hotspot = { 9, 26 },
      },
      flag = {
         pictures = path.list_files (dirname .. "images/frisians/flag_??.png"),
         hotspot = { 10, 39 },
         fps = 10,
      }
   },

   -- Image file paths for this tribe's road textures
   roads = {
      busy = {
         "tribes/images/frisians/roadt_busy.png",
      },
      normal = {
         "tribes/images/frisians/roadt_normal_00.png",
         "tribes/images/frisians/roadt_normal_01.png",
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
         "thatch_reed",
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
      "reed_tiny",
      "reed_small",
      "reed_medium",
      "reed_ripe",
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
      "field_medium",
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
      "frisians_weaving_mill",
      "frisians_shipyard",

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

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      "Amrum",
      "Föhr",
      "Gröde",
      "Habel",
      "Hamburger Hallig",
      "Hooge",
      "Japsand",
      "Kormoransand",
      "Langeneß",
      "Norderoog",
      "Norderoogsand",
      "Nordstrand",
      "Nordstrandischmoor",
      "Oland",
      "Pellworm",
      "Süderoog",
      "Süderoogsand",
      "Südfall",
      "Sylt",
   },

   -- Special types
   builder = "frisians_builder",
   carrier = "frisians_carrier",
   carrier2 = "frisians_reindeer",
   geologist = "frisians_geologist",
   soldier = "frisians_soldier",
   ship = "frisians_ship",
   port = "frisians_port",
   barracks = "frisians_barracks",
   ironore = "iron_ore",
   rawlog = "log",
   refinedlog = "brick",
   granite = "granite",
}
