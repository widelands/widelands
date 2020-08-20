tribes = wl.Tribes()
image_dirname = path.dirname(__file__) .. "images/amazons/"

tribes:new_tribe {
   name = "amazons",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {5, 35} },
      bridge_normal_e = { hotspot = {-2, 11} },
      bridge_busy_e = { hotspot = {-2, 11} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 6} },
      bridge_busy_sw = { hotspot = {36, 3} }
   },
   spritesheets = {
      flag = {
         hotspot = {26, 39},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 5
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
         "tribes/images/amazons/waterway_0.png",
      },
   },

   resource_indicators = {
      [""] = {
         [0] = "amazons_resi_none",
      },
      coal = {
         [100] = "amazons_resi_none",
      },
      iron = {
         [100] = "amazons_resi_none",
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
         "balsa",
         "ironwood",
         "rubber",
         "liana",
         "rope",
      },
      {
         -- Food
          "water",
          "cassavaroot",
          "cocoa_beans",
          "amazons_bread",
          "chocolate",
          "fish",
          "meat",
          "ration",
      },
      {
         -- Mining
          "gold_dust",
          "gold",
          "charcoal"
      },
      {
         -- Tools
          "pick",
          "felling_ax",
          "shovel",
          "hammer",
          "machete",
          "spear_wooden",
          "chisel",
          "kitchen_tools",
          "needles",
          "stonebowl",
      },
      {
         -- Weapons & Armor
          "spear_stone_tipped",
          "spear_hardened",
          "armor_wooden",
          "helmet_wooden",
          "warriors_coat",
          "tunic",
          "vest_padded",
          "protector_padded",
          "boots_sturdy",
          "boots_swift",
          "boots_hero",
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         "amazons_carrier",
         "amazons_ferry",
         "amazons_tapir",
         "amazons_tapir_breeder"
      },
      {
         -- Building Materials
         "amazons_stonecutter",
         "amazons_woodcutter",
         "amazons_woodcutter_master",
         "amazons_jungle_preserver",
         "amazons_jungle_master",
         "amazons_liana_cutter",
         "amazons_builder",
         "amazons_dressmaker",
         "amazons_shipwright"
      },
      {
         -- Food
         "amazons_hunter_gatherer",
         "amazons_cassava_farmer",
         "amazons_cocoa_farmer",
         "amazons_cook",
         "amazons_wilderness_keeper",
      },
      {
         -- Mining
         "amazons_geologist",
         "amazons_charcoal_burner",
         "amazons_gold_smelter",
         "amazons_gold_digger",
         "amazons_stonecarver"
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
      "cassavarootfield_tiny",
      "cassavarootfield_small",
      "cassavarootfield_medium",
      "cassavarootfield_ripe",
      "cassavarootfield_harvested",
      "cocoa_tree_sapling",
      "cocoa_tree_pole",
      "cocoa_tree_mature",
      "cocoa_tree_old",
      "amazons_resi_none",
      "amazons_resi_water",
      "amazons_resi_gold_1",
      "amazons_resi_stones_1",
      "amazons_resi_gold_2",
      "amazons_resi_stones_2",
      "amazons_shipconstruction",
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      "amazons_headquarters",
      "amazons_warehouse",
      "amazons_port",

      -- Small
      "amazons_stonecutters_hut",
      "amazons_woodcutters_hut",
      "amazons_junglepreservers_hut",
      "amazons_hunter_gatherers_hut",
      "amazons_liana_cutters_hut",
      "amazons_water_gatherers_hut",
      "amazons_rare_trees_woodcutters_hut",
      "amazons_wilderness_keepers_tent",
      "amazons_scouts_hut",

      -- Medium
      "amazons_rope_weaver_booth",
      "amazons_furnace",
      "amazons_rare_tree_plantation",
      "amazons_stonecarvery",
      "amazons_dressmakery",
      "amazons_charcoal_kiln",
      "amazons_cassava_root_cooker",
      "amazons_chocolate_brewery",
      "amazons_food_preserver",
      "amazons_youth_gathering",
      "amazons_gardening_center",

      -- Big
      "amazons_tapir_farm",
      "amazons_cassava_root_plantation",
      "amazons_cocoa_farm",

      -- Mines
      "amazons_stonemine",
      "amazons_gold_digger_dwelling",

      -- Training Sites
      "amazons_warriors_gathering",
      "amazons_training_glade",

      -- Military Sites
      "amazons_patrol_post",
      "amazons_treetop_sentry",
      "amazons_warriors_dwelling",
      "amazons_tower",
      "amazons_observation_tower",
      "amazons_fortress",
      "amazons_fortification",

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      "amazons_ferry_yard",
      "amazons_shipyard",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      -- TRANSLATORS: This Barbarian ship is named after a river
      pgettext("shipname", "Orinoco"),
      -- TRANSLATORS: This Barbarian ship is named after a river
      pgettext("shipname", "Amazonas"),
      -- TRANSLATORS: This Barbarian ship is named after a river
      pgettext("shipname", "Abacaxis"),
      -- TRANSLATORS: This Barbarian ship is named after a river
      pgettext("shipname", "Anchicaya"),
      -- TRANSLATORS: This Barbarian ship is named after a river
      pgettext("shipname", "Guaitara"),
      -- TRANSLATORS: This Barbarian ship is named after a river
      pgettext("shipname", "Iscuande"),
      -- TRANSLATORS: This Barbarian ship is named after a river
      pgettext("shipname", "Putumayo"),
   },

   -- Special types
   builder = "amazons_builder",
   carrier = "amazons_carrier",
   carrier2 = "amazons_tapir",
   geologist = "amazons_geologist",
   soldier = "amazons_soldier",
   ship = "amazons_ship",
   ferry = "amazons_ferry",
   port = "amazons_port",

}
