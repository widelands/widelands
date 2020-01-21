image_dirname = path.dirname(__file__) .. "images/empire/"

animations = {}
add_animation(animations, "frontier", image_dirname, "frontier", {1, 19})
add_animation(animations, "flag", image_dirname, "flag", {14, 38}, 10)
add_animation(animations, "bridge_normal_e", image_dirname, "bridge_normal_e", {-2, 12})
add_animation(animations, "bridge_busy_e", image_dirname, "bridge_busy_e", {-2, 12})
add_animation(animations, "bridge_normal_se", image_dirname, "bridge_normal_se", {5, 2})
add_animation(animations, "bridge_busy_se", image_dirname, "bridge_busy_se", {5, 2})
add_animation(animations, "bridge_normal_sw", image_dirname, "bridge_normal_sw", {36, 3})
add_animation(animations, "bridge_busy_sw", image_dirname, "bridge_busy_sw", {36, 3})

tribes:new_tribe {
   name = "empire",
   animations = animations,

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
         "tribes/images/empire/waterway_0.png",
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
         "granite",
         "log",
         "planks",
         "wool",
         "cloth"
      },
      {
         -- Food
         "fish",
         "meat",
         "water",
         "wheat",
         "flour",
         "empire_bread",
         "beer",
         "grape",
         "wine",
         "ration",
         "meal"
      },
      {
         -- Mining
         "marble",
         "marble_column",
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
         "saw",
         "shovel",
         "hammer",
         "fishing_rod",
         "hunting_spear",
         "scythe",
         "bread_paddle",
         "basket",
         "kitchen_tools",
         "fire_tongs"
      },
      {
         -- Weapons & Armor
         "spear_wooden",
         "spear",
         "spear_advanced",
         "spear_heavy",
         "spear_war",
         "armor_helmet",
         "armor",
         "armor_chain",
         "armor_gilded"
      }
   },
   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         "empire_carrier",
         "empire_ferry",
         "empire_donkey",
         "empire_donkeybreeder"
      },
      {
         -- Building Materials
         "empire_stonemason",
         "empire_carpenter",
         "empire_lumberjack",
         "empire_forester",
         "empire_builder",
         "empire_shepherd",
         "empire_weaver",
         "empire_shipwright"
      },
      {
         -- Food
         "empire_fisher",
         "empire_hunter",
         "empire_farmer",
         "empire_miller",
         "empire_baker",
         "empire_brewer",
         "empire_vinefarmer",
         "empire_vintner",
         "empire_pigbreeder",
         "empire_innkeeper"
      },
      {
         -- Mining
         "empire_geologist",
         "empire_miner",
         "empire_miner_master",
         "empire_charcoal_burner",
         "empire_smelter"
      },
      {
         -- Tools
         "empire_toolsmith"
      },
      {
         -- Military
         "empire_recruit",
         "empire_soldier",
         "empire_trainer",
         "empire_weaponsmith",
         "empire_armorsmith",
         "empire_scout"
      },

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
      "Adriaitic Sea",
      "Alexandria",
      "Amalea",
      "Amolius",
      "Apollo",
      "Augustus",
      "Azurea Sea",
      "Baltic Sea",
      "Bellona",
      "Ben Hur",
      "Bounty",
      "Brutus",
      "Caligula",
      "Camilla",
      "Carrara",
      "Ceasar",
      "Cicero",
      "Claudius",
      "Diana",
      "Domitianus",
      "Empire Glory",
      "Faustulus",
      "Galba",
      "Gallus",
      "Janus",
      "Julius Caesar",
      "Juno",
      "Juno",
      "Jupiter",
      "Jupiter",
      "Latium",
      "Latonia",
      "Leonardo",
      "Liguria",
      "Lutius",
      "Marble",
      "Mars",
      "Mars",
      "Minerva",
      "Neptune",
      "Neptune",
      "Neptune",
      "Nero",
      "Nerva",
      "Octavianus",
      "Otho",
      "Pluto",
      "Porto Bello",
      "Quirinus",
      "Remus",
      "Romulus",
      "Saledus",
      "Saturn",
      "Siena",
      "Sparta",
      "Thyrrenia",
      "Tiberius",
      "Titus",
      "Trajan",
      "Venus",
      "Venus",
      "Vespasianus",
      "Vesta",
      "Vesta",
      "Vine",
      "Vitellius",
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
   ironore = "iron_ore",
   rawlog = "log",
   refinedlog = "planks",
   granite = "granite",
}
