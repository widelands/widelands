dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "frisians",

   animations = {
      frontier = {
         pictures = path.list_files(dirname .. "images/frisians/frontier_??.png"),
         hotspot = { 12, 28 },
      },
      flag = {
         pictures = path.list_files(dirname .. "images/frisians/flag_??.png"),
         hotspot = { 13, 36 },
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
          "honey",
          "water",
          "fruit",
          "fish",
          "meat",
          "smoked_fish",
          "smoked_meat",
          "barley",
          "bread_frisians",
          "sweetbread",
          "beer",
          "mead",
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
          "scrap_metal_iron",
          "scrap_metal_mixed",
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
          "sword_basic",
          "sword_long",
          "sword_curved",
          "sword_double",
          "fur_clothes",
          "fur_clothes_silver",
          "fur_clothes_golden",
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
         "frisians_builder",
         "frisians_clay_burner",
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
         "frisians_tanner",
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
         "frisians_blacksmith"
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
      "bush_tiny",
      "bush_small",
      "bush_medium",
      "bush_ripe",
      "field_tiny",
      "field_small",
      "field_medium",
      "field_ripe",
      "field_harvested",
      "reed_tiny",
      "reed_small",
      "reed_medium",
      "reed_ripe",
      "resi_coal1",
      "resi_coal2",
      "resi_gold1",
      "resi_gold2",
      "resi_iron1",
      "resi_iron2",
      "resi_none",
      "resi_water1",
      "resi_stones1",
      "resi_stones2",
      "frisians_shipconstruction",
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
      "frisians_well",
      "frisians_clay_burners_house",
      "frisians_berry_farm",
      "frisians_collectors_house",
      "frisians_beekeepers_house",
      "frisians_scouts_house",

      -- Medium
      "frisians_reed_farm",
      "frisians_brick_burners_house",
      "frisians_furnace",
      "frisians_recycling_centre",
      "frisians_blacksmithy",
      "frisians_armour_smithy",
      "frisians_seamstress",
      "frisians_seamstress_master",
      "frisians_charcoal_kiln",
      "frisians_tannery",
      "frisians_bakery",
      "frisians_sweetbread_bakery",
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
      "frisians_aqua_farm",

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
      "frisians_sentinel",
      "frisians_outpost",
      "frisians_tower",
      "frisians_fortress",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      "Hooge",
      "Gröde",
      "Langeneß",
      "Oland",
      "Süderoog",
      "Norderoog",
      "Süderoogsand",
      "Norderoogsand",
      "Japsand",
      "Südfall",
      "Nordstrandischmoor",
      "Habel",
      "Hamburger Hallig",
      "Föhr",
      "Amrum",
      "Sylt",
      "Pellworm",
   },

   -- Special types
   builder = "frisians_builder",
   carrier = "frisians_carrier",
   carrier2 = "frisians_reindeer",
   geologist = "frisians_geologist",
   soldier = "frisians_soldier",
   ship = "frisians_ship",
   headquarters = "frisians_headquarters",
   port = "frisians_port",
   barracks = "frisians_barracks",
   bakery = "frisians_bakery",
   ironore = "iron_ore",
   rawlog = "log",
   refinedlog = "brick",
   granite = "granite",
}
