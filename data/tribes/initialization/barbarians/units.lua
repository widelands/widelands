tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes")

tribes:new_tribe {
   name = "barbarians",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {1, 19} },
      bridge_normal_e = { hotspot = {-1, 13} },
      bridge_busy_e = { hotspot = {-1, 13} },
      bridge_normal_se = { hotspot = {8, 3} },
      bridge_busy_se = { hotspot = {8, 3} },
      bridge_normal_sw = { hotspot = {41, 3} },
      bridge_busy_sw = { hotspot = {41, 3} }
   },
   spritesheets = {
      flag = {
         fps = 5,
         frames = 16,
         columns = 4,
         rows = 4,
         hotspot = { 11, 39 }
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
         [0] = "barbarians_resi_none",
      },
      coal = {
         [10] = "barbarians_resi_coal_1",
         [20] = "barbarians_resi_coal_2",
      },
      iron = {
         [10] = "barbarians_resi_iron_1",
         [20] = "barbarians_resi_iron_2",
      },
      gold = {
         [10] = "barbarians_resi_gold_1",
         [20] = "barbarians_resi_gold_2",
      },
      stones = {
         [10] = "barbarians_resi_stones_1",
         [20] = "barbarians_resi_stones_2",
      },
      water = {
         [100] = "barbarians_resi_water",
      },
   },

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
      {
         -- Building Materials
         {
            name = "granite",
            default_target_quantity = 20,
            preciousness = 5
         },
         {
            name = "log",
            preciousness = 14
         },
         {
            name = "blackwood",
            default_target_quantity = 40,
            preciousness = 10
         },
         {
            name = "grout",
            default_target_quantity = 10,
            preciousness = 5
         },
         {
            name = "reed",
            preciousness = 5
         },
         {
            name = "cloth",
            default_target_quantity = 10,
            preciousness = 0
         }
      },
      {
         -- Food
         {
            name = "fish",
            preciousness = 3
         },
         {
            name = "meat",
            preciousness = 3
         },
         {
            name = "water",
            preciousness = 8
         },
         {
            name = "wheat",
            preciousness = 12
         },
         {
            name = "barbarians_bread",
            default_target_quantity = 20,
            preciousness = 4
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness =  2
         },
         {
            name = "beer_strong",
            default_target_quantity = 15,
            preciousness = 2
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 5
         },
         {
            name = "snack",
            default_target_quantity = 15,
            preciousness = 5
         },
         {
            name = "meal",
            default_target_quantity = 10,
            preciousness = 5
         }
      },
      {
         -- Mining
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 20
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 4
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 2
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2
         }
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 2,
            preciousness = 1
         },
         {
            name = "felling_ax",
            default_target_quantity = 5,
            preciousness = 3
         },
         {
            name = "shovel",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1
         },
         {
            name = "fishing_rod",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "hunting_spear",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "scythe",
            default_target_quantity = 1,
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
            default_target_quantity = 1,
            preciousness = 0,
         }
      },
      {
         -- Weapons & Armor
         {
            name = "ax",
            default_target_quantity = 30,
            preciousness = 3
         },
         {
            name = "ax_sharp",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "ax_broad",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "ax_bronze",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "ax_battle",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "ax_warriors",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "helmet",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "helmet_mask",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "helmet_warhelm",
            default_target_quantity = 1,
            preciousness = 2
         }
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         { name = "barbarians_carrier" },
         { name = "barbarians_ferry" },
         {
            name = "barbarians_ox",
            default_target_quantity = 10,
            preciousness = 2
         },
         { name = "barbarians_cattlebreeder" }
      },
      {
         -- Building Materials
         { name = "barbarians_stonemason" },
         { name = "barbarians_lumberjack" },
         { name = "barbarians_ranger" },
         { name = "barbarians_builder" },
         { name = "barbarians_lime_burner" },
         { name = "barbarians_gardener" },
         { name = "barbarians_weaver" },
         { name = "barbarians_shipwright" }
      },
      {
         -- Food
         { name = "barbarians_fisher" },
         { name = "barbarians_hunter" },
         { name = "barbarians_gamekeeper" },
         { name = "barbarians_farmer" },
         { name = "barbarians_baker" },
         { name = "barbarians_brewer" },
         { name = "barbarians_brewer_master" },
         { name = "barbarians_innkeeper" }
      },
      {
         -- Mining
         { name = "barbarians_geologist" },
         { name = "barbarians_miner" },
         { name = "barbarians_miner_chief" },
         { name = "barbarians_miner_master" },
         { name = "barbarians_charcoal_burner" },
         { name = "barbarians_smelter" }
      },
      {
         -- Tools
         { name = "barbarians_blacksmith" },
         { name = "barbarians_blacksmith_master" }
      },
      {
         -- Military
         { name = "barbarians_recruit" },
         {
            name = "barbarians_soldier",
            default_target_quantity = 10,
            preciousness = 5
         },
         { name = "barbarians_trainer" },
         { name = "barbarians_helmsmith" },
         { name = "barbarians_scout" }
      }
   },

   immovables = {
      "ashes",
      "destroyed_building",
      "wheatfield_tiny",
      "wheatfield_small",
      "wheatfield_medium",
      "wheatfield_ripe",
      "wheatfield_harvested",
      "reedfield_tiny",
      "reedfield_small",
      "reedfield_medium",
      "reedfield_ripe",
      "barbarians_resi_none",
      "barbarians_resi_water",
      "barbarians_resi_coal_1",
      "barbarians_resi_iron_1",
      "barbarians_resi_gold_1",
      "barbarians_resi_stones_1",
      "barbarians_resi_coal_2",
      "barbarians_resi_iron_2",
      "barbarians_resi_gold_2",
      "barbarians_resi_stones_2",
      "barbarians_shipconstruction",
      -- non barbarian Immovables used by the woodcutter
      "deadtree7",
      "balsa_amazons_sapling",
      "balsa_amazons_pole",
      "balsa_amazons_mature",
      "balsa_amazons_old",
      "balsa_black_amazons_sapling",
      "balsa_black_amazons_pole",
      "balsa_black_amazons_mature",
      "balsa_black_amazons_old",
      "balsa_desert_amazons_sapling",
      "balsa_desert_amazons_pole",
      "balsa_desert_amazons_mature",
      "balsa_desert_amazons_old",
      "balsa_winter_amazons_sapling",
      "balsa_winter_amazons_pole",
      "balsa_winter_amazons_mature",
      "balsa_winter_amazons_old",
      "ironwood_amazons_sapling",
      "ironwood_amazons_pole",
      "ironwood_amazons_mature",
      "ironwood_amazons_old",
      "ironwood_black_amazons_sapling",
      "ironwood_black_amazons_pole",
      "ironwood_black_amazons_mature",
      "ironwood_black_amazons_old",
      "ironwood_desert_amazons_sapling",
      "ironwood_desert_amazons_pole",
      "ironwood_desert_amazons_mature",
      "ironwood_desert_amazons_old",
      "ironwood_winter_amazons_sapling",
      "ironwood_winter_amazons_pole",
      "ironwood_winter_amazons_mature",
      "ironwood_winter_amazons_old",
      "rubber_amazons_sapling",
      "rubber_amazons_pole",
      "rubber_amazons_mature",
      "rubber_amazons_old",
      "rubber_black_amazons_sapling",
      "rubber_black_amazons_pole",
      "rubber_black_amazons_mature",
      "rubber_black_amazons_old",
      "rubber_desert_amazons_sapling",
      "rubber_desert_amazons_pole",
      "rubber_desert_amazons_mature",
      "rubber_desert_amazons_old",
      "rubber_winter_amazons_sapling",
      "rubber_winter_amazons_pole",
      "rubber_winter_amazons_mature",
      "rubber_winter_amazons_old",
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
      "barbarians_barracks",

      -- Big
      "barbarians_cattlefarm",
      "barbarians_farm",
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

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      "barbarians_ferry_yard",
      "barbarians_shipyard",
      "barbarians_weaving_mill",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      pgettext("shipname", "Agilaz"),
      pgettext("shipname", "Aslaug"),
      pgettext("shipname", "Baldr"),
      pgettext("shipname", "Bear"),
      pgettext("shipname", "Beowulf"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Boldreth"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Chat’Karuth"),
      pgettext("shipname", "Dellingr"),
      pgettext("shipname", "Fulla"),
      pgettext("shipname", "Gersemi"),
      pgettext("shipname", "Hagbard"),
      pgettext("shipname", "Heidrek"),
      pgettext("shipname", "Heimdallr"),
      pgettext("shipname", "Hnoss"),
      pgettext("shipname", "Hrothgar"),
      pgettext("shipname", "Ingeld"),
      pgettext("shipname", "Karl Hundason"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Khantrukh"),
      pgettext("shipname", "Lynx"),
      pgettext("shipname", "Mani"),
      pgettext("shipname", "Odin"),
      pgettext("shipname", "Red Fox"),
      -- TRANSLATORS: This Barbarian ship is mentioned in some lore texts
      pgettext("shipname", "Saxnot"),
      pgettext("shipname", "Sigmund"),
      -- TRANSLATORS: This Barbarian ship is named after a barbarian bard
      pgettext("shipname", "Sigurd"),
      pgettext("shipname", "Snotra"),
      pgettext("shipname", "Thor"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Thron"),
      pgettext("shipname", "Ullr"),
      pgettext("shipname", "Valdar"),
      pgettext("shipname", "Vili"),
      pgettext("shipname", "Volf"),
      pgettext("shipname", "Wild Boar"),
      pgettext("shipname", "Wolverine"),
      pgettext("shipname", "Yrsa"),
   },

   -- Special types
   builder = "barbarians_builder",
   carrier = "barbarians_carrier",
   carrier2 = "barbarians_ox",
   geologist = "barbarians_geologist",
   soldier = "barbarians_soldier",
   ship = "barbarians_ship",
   ferry = "barbarians_ferry",
   port = "barbarians_port",
}

pop_textdomain()
