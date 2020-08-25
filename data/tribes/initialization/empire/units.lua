tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes")

tribes:new_tribe {
   name = "empire",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {1, 19} },
      flag = { hotspot = {14, 38}, fps = 10 },
      bridge_normal_e = { hotspot = {-2, 12} },
      bridge_busy_e = { hotspot = {-2, 12} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 3} },
      bridge_busy_sw = { hotspot = {36, 3} }
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
         image_dirname .. "roadt_normal_02.png",
      },
      waterway = {
         image_dirname .. "waterway_0.png",
      },
   },

   resource_indicators = {
      [""] = {
         [0] = "empire_resi_none",
      },
      coal = {
         [10] = "empire_resi_coal_1",
         [20] = "empire_resi_coal_2",
      },
      iron = {
         [10] = "empire_resi_iron_1",
         [20] = "empire_resi_iron_2",
      },
      gold = {
         [10] = "empire_resi_gold_1",
         [20] = "empire_resi_gold_2",
      },
      stones = {
         [10] = "empire_resi_stones_1",
         [20] = "empire_resi_stones_2",
      },
      water = {
         [100] = "empire_resi_water",
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
            default_target_quantity = 30,
            preciousness = 0
         },
         {
            name = "log",
            preciousness = 14
         },
         {
            name = "planks",
            default_target_quantity = 20,
            preciousness = 10
         },
         {
            name = "wool",
            default_target_quantity = 10,
            preciousness = 2
         },
         {
            name = "cloth",
            default_target_quantity = 15,
            preciousness = 1
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
            default_target_quantity = 20,
            preciousness = 2
         },
         {
            name = "water",
            preciousness = 7
         },
         {
            name = "wheat",
            preciousness = 12
         },
         {
            name = "flour",
            default_target_quantity = 20,
            preciousness = 9
         },
         {
            name = "empire_bread",
            default_target_quantity = 20,
            preciousness = 7
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness = 5
         },
         {
            name = "grape",
            preciousness = 10
         },
         {
            name = "wine",
            preciousness = 8
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 4
         },
         {
            name = "meal",
            default_target_quantity = 20,
            preciousness = 4
         }
      },
      {
         -- Mining
         {
            name = "marble",
            default_target_quantity = 30,
            preciousness = 3
         },
         {
            name = "marble_column",
            default_target_quantity = 10,
            preciousness = 5
         },
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 10
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
            default_target_quantity = 3,
            preciousness = 1
         },
         {
            name = "saw",
            default_target_quantity = 1,
            preciousness = 0
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
            name = "basket",
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
            preciousness = 0
         }
      },
      {
         -- Weapons & Armor
         {
            name = "spear_wooden",
            default_target_quantity = 30,
            preciousness = 1
         },
         {
            name = "spear",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "spear_advanced",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "spear_heavy",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "spear_war",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "armor_helmet",
            default_target_quantity = 30,
            preciousness = 2
         },
         {
            name = "armor",
            default_target_quantity = 1,
            preciousness = 0,
         },
         {
            name = "armor_chain",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "armor_gilded",
            default_target_quantity = 1,
            preciousness = 1
         }
      }
   },
   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         { name = "empire_carrier" },
         { name = "empire_ferry" },
         {
            name = "empire_donkey",
            default_target_quantity = 10,
            preciousness = 2
         },
         { name = "empire_donkeybreeder" }
      },
      {
         -- Building Materials
         { name = "empire_stonemason" },
         { name = "empire_carpenter" },
         { name = "empire_lumberjack" },
         { name = "empire_forester" },
         { name = "empire_builder" },
         { name = "empire_shepherd" },
         { name = "empire_weaver" },
         { name = "empire_shipwright" }
      },
      {
         -- Food
         { name = "empire_fisher" },
         { name = "empire_hunter" },
         { name = "empire_farmer" },
         { name = "empire_miller" },
         { name = "empire_baker" },
         { name = "empire_brewer" },
         { name = "empire_vinefarmer" },
         { name = "empire_vintner" },
         { name = "empire_pigbreeder" },
         { name = "empire_innkeeper" }
      },
      {
         -- Mining
         { name = "empire_geologist" },
         { name = "empire_miner" },
         { name = "empire_miner_master" },
         { name = "empire_charcoal_burner" },
         { name = "empire_smelter" }
      },
      {
         -- Tools
         { name = "empire_toolsmith" }
      },
      {
         -- Military
         { name = "empire_recruit" },
         {
            name = "empire_soldier",
            default_target_quantity = 10,
            preciousness = 5
         },
         { name = "empire_trainer" },
         { name = "empire_weaponsmith" },
         { name = "empire_armorsmith" },
         { name = "empire_scout" }
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
      "grapevine_tiny",
      "grapevine_small",
      "grapevine_medium",
      "grapevine_ripe",
      "empire_resi_none",
      "empire_resi_water",
      "empire_resi_coal_1",
      "empire_resi_iron_1",
      "empire_resi_gold_1",
      "empire_resi_stones_1",
      "empire_resi_coal_2",
      "empire_resi_iron_2",
      "empire_resi_gold_2",
      "empire_resi_stones_2",
      "empire_shipconstruction",
      -- non empire Immovables used by the woodcutter
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
      "empire_headquarters",
      "empire_headquarters_shipwreck",
      "empire_warehouse",
      "empire_port",

      -- Small
      "empire_quarry",
      "empire_lumberjacks_house",
      "empire_foresters_house",
      "empire_fishers_house",
      "empire_hunters_house",
      "empire_well",
      "empire_scouts_house",

      -- Medium
      "empire_stonemasons_house",
      "empire_sawmill",
      "empire_mill",
      "empire_bakery",
      "empire_brewery",
      "empire_vineyard",
      "empire_winery",
      "empire_tavern",
      "empire_inn",
      "empire_charcoal_kiln",
      "empire_smelting_works",
      "empire_toolsmithy",
      "empire_armorsmithy",
      "empire_barracks",

      -- Big
      "empire_donkeyfarm",
      "empire_farm",
      "empire_piggery",
      "empire_sheepfarm",
      "empire_weaving_mill",
      "empire_weaponsmithy",


      -- Mines
      "empire_marblemine",
      "empire_marblemine_deep",
      "empire_coalmine",
      "empire_coalmine_deep",
      "empire_ironmine",
      "empire_ironmine_deep",
      "empire_goldmine",
      "empire_goldmine_deep",

      -- Training Sites
      "empire_arena",
      "empire_colosseum",
      "empire_trainingcamp",

      -- Military Sites
      "empire_blockhouse",
      "empire_sentry",
      "empire_outpost",
      "empire_barrier",
      "empire_tower",
      "empire_fortress",
      "empire_castle",

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      "empire_ferry_yard",
      "empire_shipyard",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      pgettext("shipname", "Adriatic Sea"),
      pgettext("shipname", "Alexandria"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Amalea"),
      pgettext("shipname", "Amolius"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Apollo"),
      pgettext("shipname", "Augustus"),
      pgettext("shipname", "Azurea Sea"),
      pgettext("shipname", "Baltic Sea"),
      pgettext("shipname", "Bellona"),
      pgettext("shipname", "Ben Hur"),
      pgettext("shipname", "Bounty"),
      pgettext("shipname", "Brutus"),
      pgettext("shipname", "Caligula"),
      pgettext("shipname", "Camilla"),
      pgettext("shipname", "Carrara"),
      pgettext("shipname", "Caesar"),
      pgettext("shipname", "Cicero"),
      pgettext("shipname", "Claudius"),
      pgettext("shipname", "Diana"),
      pgettext("shipname", "Domitianus"),
      pgettext("shipname", "Empire Glory"),
      pgettext("shipname", "Faustulus"),
      pgettext("shipname", "Galba"),
      pgettext("shipname", "Gallus"),
      pgettext("shipname", "Ianus"),
      pgettext("shipname", "Iulius Caesar"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Iuno"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Iupiter"),
      pgettext("shipname", "Latium"),
      pgettext("shipname", "Latonia"),
      pgettext("shipname", "Leonardo"),
      pgettext("shipname", "Liguria"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Lutius"),
      pgettext("shipname", "Marble"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Mars"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Minerva"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Neptune"),
      pgettext("shipname", "Nero"),
      pgettext("shipname", "Nerva"),
      pgettext("shipname", "Octavianus"),
      pgettext("shipname", "Otho"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Pluto"),
      pgettext("shipname", "Portus Bellus"),
      pgettext("shipname", "Quirinus"),
      pgettext("shipname", "Remus"),
      pgettext("shipname", "Romulus"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Saledus"),
      -- TRANSLATORS: This Empire ship is named after a Roman deity
      pgettext("shipname", "Saturn"),
      pgettext("shipname", "Siena"),
      pgettext("shipname", "Sparta"),
      pgettext("shipname", "Thyrrenia"),
      pgettext("shipname", "Tiberius"),
      pgettext("shipname", "Titus"),
      pgettext("shipname", "Traianus"),
      pgettext("shipname", "Venus"),
      pgettext("shipname", "Vespasianus"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Vesta"),
      pgettext("shipname", "Vinea"),
      pgettext("shipname", "Vitellius"),
   },

   -- Special types
   builder = "empire_builder",
   carrier = "empire_carrier",
   carrier2 = "empire_donkey",
   geologist = "empire_geologist",
   soldier = "empire_soldier",
   ship = "empire_ship",
   ferry = "empire_ferry",
   port = "empire_port",
}

pop_textdomain()
