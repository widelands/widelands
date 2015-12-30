dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "barbarians",

   animations = {
      frontier = {
         pictures = path.list_files(dirname .. "images/barbarians/frontier_??.png"),
         hotspot = { 1, 19 },
      },
      flag = {
         pictures = path.list_files(dirname .. "images/barbarians/flag_??.png"),
         hotspot = { 10, 38 },
         fps = 5
      }
   },

   -- Image file paths for this tribe's road textures
   roads = {
      busy = {
         "tribes/images/barbarians/roadt_busy.png",
      },
      normal = {
         "tribes/images/barbarians/roadt_normal_00.png",
         "tribes/images/barbarians/roadt_normal_01.png",
      },
   },

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
      {
         -- Building Materials
         "granite",
         "log",
         "blackwood",
         "grout",
         "thatch_reed",
         "cloth"
      },
      {
         -- Food
          "fish",
          "meat",
          "water",
          "wheat",
          "barbarians_bread",
          "beer",
          "beer_strong",
          "ration",
          "snack",
          "meal"
      },
      {
         -- Mining
          "coal",
          "iron_ore",
          "iron",
          "gold_ore",
          "gold"
      },
      {
         -- Tools
          "pick",
          "felling_ax",
          "shovel",
          "hammer",
          "fishing_rod",
          "hunting_spear",
          "scythe",
          "bread_paddle",
          "kitchen_tools",
          "fire_tongs"
      },
      {
         -- Weapons & Armor
          "ax",
          "ax_sharp",
          "ax_broad",
          "ax_bronze",
          "ax_battle",
          "ax_warriors",
          "helmet",
          "helmet_mask",
          "helmet_warhelm"
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         "barbarians_carrier",
         "barbarians_ox",
         "barbarians_cattlebreeder"
      },
      {
         -- Building Materials
         "barbarians_stonemason",
         "barbarians_lumberjack",
         "barbarians_ranger",
         "barbarians_builder",
         "barbarians_lime_burner",
         "barbarians_gardener",
         "barbarians_weaver",
         "barbarians_shipwright"
      },
      {
         -- Food
         "barbarians_fisher",
         "barbarians_hunter",
         "barbarians_gamekeeper",
         "barbarians_farmer",
         "barbarians_baker",
         "barbarians_brewer",
         "barbarians_brewer_master",
         "barbarians_innkeeper"
      },
      {
         -- Mining
         "barbarians_geologist",
         "barbarians_miner",
         "barbarians_miner_chief",
         "barbarians_miner_master",
         "barbarians_charcoal_burner",
         "barbarians_smelter"
      },
      {
         -- Tools
         "barbarians_blacksmith",
         "barbarians_blacksmith_master"
      },
      {
         -- Military
         "barbarians_soldier",
         "barbarians_trainer",
         "barbarians_helmsmith",
         "barbarians_scout"
      }
   },

   immovables = {
      "ashes",
      "destroyed_building",
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
      "barbarians_shipconstruction",
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      "barbarians_headquarters",
      "barbarians_headquarters_interim",
      "barbarians_warehouse",
      "barbarians_port",

      -- Small
      "barbarians_quarry",
      "barbarians_lumberjacks_hut",
      "barbarians_rangers_hut",
      "barbarians_fishers_hut",
      "barbarians_hunters_hut",
      "barbarians_gamekeepers_hut",
      "barbarians_well",
      "barbarians_scouts_hut",

      -- Medium
      "barbarians_wood_hardener",
      "barbarians_reed_yard",
      "barbarians_lime_kiln",
      "barbarians_bakery",
      "barbarians_micro_brewery",
      "barbarians_brewery",
      "barbarians_tavern",
      "barbarians_inn",
      "barbarians_big_inn",
      "barbarians_charcoal_kiln",
      "barbarians_smelting_works",
      "barbarians_metal_workshop",
      "barbarians_warmill",
      "barbarians_ax_workshop",
      "barbarians_shipyard",

      -- Big
      "barbarians_cattlefarm",
      "barbarians_farm",
      "barbarians_weaving_mill",
      "barbarians_helmsmithy",

      -- Mines
      "barbarians_granitemine",
      "barbarians_coalmine",
      "barbarians_coalmine_deep",
      "barbarians_coalmine_deeper",
      "barbarians_ironmine",
      "barbarians_ironmine_deep",
      "barbarians_ironmine_deeper",
      "barbarians_goldmine",
      "barbarians_goldmine_deep",
      "barbarians_goldmine_deeper",

      -- Training Sites
      "barbarians_battlearena",
      "barbarians_trainingcamp",

      -- Military Sites
      "barbarians_sentry",
      "barbarians_barrier",
      "barbarians_tower",
      "barbarians_fortress",
      "barbarians_citadel",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   -- Special types
   builder = "barbarians_builder",
   carrier = "barbarians_carrier",
   carrier2 = "barbarians_ox",
   geologist = "barbarians_geologist",
   soldier = "barbarians_soldier",
   ship = "barbarians_ship",
   headquarters = "barbarians_headquarters",
   port = "barbarians_port",
}
