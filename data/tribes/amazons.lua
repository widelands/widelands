image_dirname = path.dirname(__file__) .. "images/amazons/"

animations = {}
add_animation(animations, "frontier", image_dirname, "frontier", {9, 26})
add_animation(animations, "flag", image_dirname, "flag", {10, 39}, 10)

tribes:new_tribe {
   name = "amazons",
   animations = animations,

   -- Image file paths for this tribe's road textures
   roads = {
      busy = {
         image_dirname .. "roadt_busy.png",
      },
      normal = {
         image_dirname .. "roadt_normal_00.png",
         image_dirname .. "roadt_normal_01.png",
      },
   },

   resource_indicators = {
      [""] = {
         [0] = "amazons_resi_none",
      },
      coal = {
         [10] = "amazons_resi_coal_1",
         [20] = "amazons_resi_coal_2",
      },
      iron = {
         [10] = "amazons_resi_iron_1",
         [20] = "amazons_resi_iron_2",
      },
      gold = {
         [10] = "amazons_resi_gold_1",
         [20] = "amazons_resi_gold_2",
      },
      stones = {
         [10] = "amazons_resi_stones_1",
         [20] = "amazons_resi_stones_2",
      },
      water = {
         [100] = "amazons_resi_water",
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
          "gold_ore",
          "gold",
      },
      {
         -- Tools
          "pick",
          "felling_ax",
          "shovel",
          "hammer",
          "machete",
          "hunting_spear",
          "chisel",
          "kitchen_tools",
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
         "amazons_carrier",
         "amazons_tapir",
         "amazons_tapir_breeder"
      },
      {
         -- Building Materials
         "amazons_stonemason",
         "amazons_woodcutter",
         "amazons_forester",
         "amazons_claydigger",
         "amazons_brickmaker",
         "amazons_builder",
         "amazons_reed_farmer",
         "amazons_seamstress",
         "amazons_seamstress_master",
         "amazons_shipwright"
      },
      {
         -- Food
         "amazons_fisher",
         "amazons_hunter",
         "amazons_farmer",
         "amazons_berry_farmer",
         "amazons_fruit_collector",
         "amazons_smoker",
         "amazons_beekeeper",
         "amazons_baker",
         "amazons_baker_master",
         "amazons_brewer",
         "amazons_brewer_master",
         "amazons_landlady"
      },
      {
         -- Mining
         "amazons_geologist",
         "amazons_miner",
         "amazons_miner_master",
         "amazons_charcoal_burner",
         "amazons_smelter",
         "amazons_blacksmith",
         "amazons_blacksmith_master"
      },
      {
         -- Military
         "amazons_soldier",
         "amazons_trainer",
         "amazons_scout"
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
      "reedfield_tiny",
      "reedfield_small",
      "reedfield_medium",
      "reedfield_ripe",
      "amazons_resi_none",
      "amazons_resi_water",
      "amazons_resi_coal_1",
      "amazons_resi_iron_1",
      "amazons_resi_gold_1",
      "amazons_resi_stones_1",
      "amazons_resi_coal_2",
      "amazons_resi_iron_2",
      "amazons_resi_gold_2",
      "amazons_resi_stones_2",
      "amazons_shipconstruction",
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
      "amazons_headquarters",
      "amazons_warehouse",
      "amazons_port",

      -- Small
      "amazons_quarry",
      "amazons_woodcutters_house",
      "amazons_foresters_house",
      "amazons_hunters_house",
      "amazons_fishers_house",
      "amazons_reed_farm",
      "amazons_well",
      "amazons_clay_pit",
      "amazons_berry_farm",
      "amazons_collectors_house",
      "amazons_beekeepers_house",
      "amazons_aqua_farm",
      "amazons_scouts_house",

      -- Medium
      "amazons_brick_kiln",
      "amazons_furnace",
      "amazons_recycling_center",
      "amazons_blacksmithy",
      "amazons_armor_smithy_small",
      "amazons_armor_smithy_large",
      "amazons_sewing_room",
      "amazons_tailors_shop",
      "amazons_charcoal_kiln",
      "amazons_smokery",
      "amazons_bakery",
      "amazons_honey_bread_bakery",
      "amazons_brewery",
      "amazons_mead_brewery",
      "amazons_tavern",
      "amazons_drinking_hall",
      "amazons_barracks",
      "amazons_weaving_mill",
      "amazons_shipyard",

      -- Big
      "amazons_reindeer_farm",
      "amazons_farm",

      -- Mines
      "amazons_rockmine",
      "amazons_rockmine_deep",
      "amazons_coalmine",
      "amazons_coalmine_deep",
      "amazons_ironmine",
      "amazons_ironmine_deep",
      "amazons_goldmine",
      "amazons_goldmine_deep",

      -- Training Sites
      "amazons_training_camp",
      "amazons_training_arena",

      -- Military Sites
      "amazons_wooden_tower",
      "amazons_wooden_tower_high",
      "amazons_sentinel",
      "amazons_outpost",
      "amazons_tower",
      "amazons_fortress",

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
   builder = "amazons_builder",
   carrier = "amazons_carrier",
   carrier2 = "amazons_reindeer",
   geologist = "amazons_geologist",
   soldier = "amazons_soldier",
   ship = "amazons_ship",
   port = "amazons_port",
   ironore = "iron_ore",
   rawlog = "log",
   refinedlog = "brick",
   granite = "granite",
}
