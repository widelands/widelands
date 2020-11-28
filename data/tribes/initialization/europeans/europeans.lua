-- RST
-- .. _lua_tribes_<tribename>.lua:
--
-- <tribename>.lua
-- ---------------------
--
-- This file contains all the units for a tribe.
--
-- .. function:: new_tribe{table}
--
--    This function adds all units to a tribe.
--
--    :arg table: This table contains all the data that the game engine will add to the tribe.
--                It contains the following entries:
--
--    **name**: A string containing the internal name of the tribe.
--
--    **animations**: Global animations. Contains subtables for ``frontier`` and ``flag``. Each animation needs the parameters ``pictures`` (table of filenames) and ``hotspot`` (2 integer coordinates), and may also define ``fps`` (integer frames per second).
--
--    **bridges**: Contains animations for ``normal_e``, ``normal_se``, ``normal_sw``, ``busy_e``, ``busy_se`` and ``busy_sw``.
--
--    **bridge_height**: The height in pixels of each bridge at it's summit at 1x scale.
--
--    **roads**: The file paths for the tribe's road textures in 3 subtables ``busy``, ``normal`` and ``waterway``.
--
--    **resource_indicators**: The names for the resource indicators. This table contains a subtable for each resource name plus a subtable named "" for no resources. Each subtable is an array, in which the index of each entry is the highest amount of resources the indicator may indicate.
--
--    **wares_order**: This defines all the wares that this tribe uses and their display order in the user interface. Each subtable defines a column in the user interface.
--
--    **workers_order**:  This defines all the workers that this tribe uses and their display order in the user interface. Each subtable defines a column in the user interface.
--
--    **immovables**: This defines all the immovables that this tribe uses.
--
--    **buildings**: This defines all the buildings that this tribe uses and their display order in the user interface.
--
--    **ship_names**: A list of strings with ship names presented to the user - be creative :)
--
--    **builder**:  The internal name of the tribe's builder. This unit needs to be defined in the ``workers_order`` table too.
--
--    **carrier**:  The internal name of the tribe's carrier. This unit needs to be defined in the ``workers_order`` table too.
--
--    **carrier2**:  The internal name of the tribe's beast of burden. This unit needs to be defined in the ``workers_order`` table too.
--
--    **geologist**:  The internal name of the tribe's geologist. This unit needs to be defined in the ``workers_order`` table too.
--
--    **soldier**: The internal name of the tribe's soldier. This unit needs to be defined in the ``workers_order`` table too.
--
--    **ship**: The internal name of the tribe's ship.
--
--    **ferry**: The internal name of the tribe's ferry.
--
--    **port**: The internal name of the tribe's port building. This unit needs to be defined in the ``buildings`` table too.
--
--    **toolbar**: *Optional*. Replace the default toolbar images with these custom images. Example:
--
--    .. code-block:: lua
--
--       toolbar = {
--          left_corner = dirname .. "images/atlanteans/toolbar_left_corner.png",
--          left = dirname .. "images/atlanteans/toolbar_left.png", -- Will be tiled
--          center = dirname .. "images/atlanteans/toolbar_center.png",
--          right = dirname .. "images/atlanteans/toolbar_right.png", -- Will be tiled
--          right_corner = dirname .. "images/atlanteans/toolbar_right_corner.png"
--       }
--

dirname = path.dirname(__file__)

image_dirname = path.dirname(__file__) .. "images/europeans/"

animations = {}
add_animation(animations, "frontier", image_dirname, "frontier", {3, 12})
--add_animation(animations, "flag", image_dirname, "flag", {15, 35}, 10)
add_animation(animations, "bridge_normal_e", image_dirname, "bridge_normal_e", {-2, 11})
add_animation(animations, "bridge_busy_e", image_dirname, "bridge_busy_e", {-2, 11})
add_animation(animations, "bridge_normal_se", image_dirname, "bridge_normal_se", {5, 2})
add_animation(animations, "bridge_busy_se", image_dirname, "bridge_busy_se", {5, 2})
add_animation(animations, "bridge_normal_sw", image_dirname, "bridge_normal_sw", {36, 6})
add_animation(animations, "bridge_busy_sw", image_dirname, "bridge_busy_sw", {36, 3})


tribes:new_tribe {
   name = "europeans",
   animations = animations,
   animation_directory = image_dirname,
   spritesheets = {
      flag = {
         directory = image_dirname,
         basename = "flag",
         fps = 5,
         frames = 16,
         columns = 4,
         rows = 4,
         hotspot = { 11, 39 }
      }
   },

   toolbar = {
      left_corner = image_dirname .. "toolbar_left_corner.png",
      left = image_dirname .. "toolbar_left.png",
      center = image_dirname .. "toolbar_center.png",
      right = image_dirname .. "toolbar_right.png",
      right_corner = image_dirname .. "toolbar_right_corner.png"
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
         [0] = "atlanteans_resi_none",
      },
      coal = {
         [10] = "atlanteans_resi_coal_1",
         [20] = "atlanteans_resi_coal_2",
      },
      iron = {
         [10] = "atlanteans_resi_iron_1",
         [20] = "atlanteans_resi_iron_2",
      },
      gold = {
         [10] = "atlanteans_resi_gold_1",
         [20] = "atlanteans_resi_gold_2",
      },
      stones = {
         [10] = "atlanteans_resi_stones_1",
         [20] = "atlanteans_resi_stones_2",
      },
      water = {
         [100] = "atlanteans_resi_water",
      },
   },

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
      {
         -- Food_1
         "fish",
         "smoked_fish",
         "meat",
         "smoked_meat",
         "corn",
         "cornmeal",
         "rye",
         "wheat",
         "flour",
         "europeans_bread",
      },
      {
         -- Food_2
         "water",
         "barley",
         "beer",
         "beer_strong",
         "grape",
         "wine",
         "fruit",
         "honey",
         "mead",
      },
      {
         -- Building Materials and Mining_1
         "log",
         "planks",
         "reed",
         "wool",
         "spider_silk",
         "spidercloth",
      },
      {
         -- Building Materials and Mining_2
         "granite",
         "grout",
         "clay",
         "brick",
         "marble",
         "marble_column",
         "quartz",
         "diamond",
      },
      {
         -- Coal, Iron, Metals
         "coal",
         "ore",
         "scrap_iron",
         "iron",
         "scrap_metal_mixed",
         "gold",
      },
      {
         -- Tools_1
         "basket",
         "hammer",
         "saw",
         "felling_ax",
         "pick",
         "shovel",
         "fire_tongs",
         "needles",
      },
      {
         -- Tools_2
         "scythe",
         "bread_paddle",
         "milking_tongs",
         "fishing_net",
         "fishing_rod",
         "hunting_bow",
         "hunting_spear",
         "hook_pole",
         "buckets",
      },
      {
         -- Military_1
         "ax",
         "spear",
         "trident_light",
         "sword_short",
         "armor_helmet",
         "spear_wooden",
         "helmet",
         "armor",
      },
      {
         -- Military_2
         "ax_sharp",
         "spear_advanced",
         "trident_long",
         "sword_long",
         "shield_steel",
         "tabard",
         "helmet_mask",
         "armor_chain",
      },
      {
         -- Military_3
         "ax_broad",
         "spear_heavy",
         "trident_steel",
         "sword_broad",
         "shield_advanced",
         "tabard_golden",
         "helmet_golden",
         "armor_gilded",
       },
      {
         -- Military_4
         "ax_battle",
         "spear_war",
         "trident_double",
         "sword_double",
         "ax_warriors",
         "trident_heavy_double",
         "helmet_warhelm",
         "gold_thread"
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers and Builders
         "europeans_carrier",
         "europeans_carrier_1",
         "europeans_carrier_2",
         "europeans_ferry",
         "europeans_builder",
         "europeans_shipwright"
      },
      {
         -- Wood
         "europeans_lumberjack",
         "europeans_woodcutter",
         "europeans_woodcutter_level_1",
         "europeans_woodcutter_level_2",
         "europeans_woodcutter_level_3",
         "europeans_ranger",
         "europeans_forester",
         "europeans_forester_level_1",
         "europeans_forester_level_2",
         "europeans_forester_level_3"
      },
      {
         -- Food
         "europeans_hunter",
         "europeans_hunter_level_1",
         "europeans_hunter_level_2",
         "europeans_hunter_level_3",
         "europeans_fisher",
         "europeans_fisher_level_1",
         "europeans_fisher_level_2",
         "europeans_fisher_level_3",
         "europeans_beekeeper"
      },
      {
         -- Processing
         "europeans_sawyer",
         "europeans_spiderbreeder",
         "europeans_shepherd",
         "europeans_weaver",
         "europeans_smoker",
         "europeans_farmer",
         "europeans_miller",
         "europeans_baker",
         "europeans_brewer"
      },
      {
         -- Mining
         "europeans_geologist",
         "europeans_miner",
         "europeans_miner_advanced",
         "europeans_miner_master",
         "europeans_stonecutter",
         "europeans_claydigger",
         "europeans_stonemason",
         "europeans_charcoal_burner",
         "europeans_smelter"
      },
      {
         -- Tools
         "europeans_toolsmith",
         "europeans_armorsmith",
         "europeans_weaponsmith"
      },
      {
         -- Military
         "europeans_recruit",
         "europeans_trainer",
         "europeans_soldier",
         "europeans_scout"
      }
   },

   immovables = {
      "ashes",
      "destroyed_building",
      "barleyfield_tiny",
      "barleyfield_small",
      "barleyfield_medium",
      "barleyfield_ripe",
      "barleyfield_harvested",
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
      "blackrootfield_tiny",
      "blackrootfield_small",
      "blackrootfield_medium",
      "blackrootfield_ripe",
      "blackrootfield_harvested",
      "cornfield_tiny",
      "cornfield_small",
      "cornfield_medium",
      "cornfield_ripe",
      "cornfield_harvested",
      "grapevine_tiny",
      "grapevine_small",
      "grapevine_medium",
      "grapevine_ripe",
      "pond_dry",
      "pond_growing",
      "pond_mature",
      "reedfield_tiny",
      "reedfield_small",
      "reedfield_medium",
      "reedfield_ripe",
      "ryefield_tiny",
      "ryefield_small",
      "ryefield_medium",
      "ryefield_ripe",
      "ryefield_harvested",
      "wheatfield_tiny",
      "wheatfield_small",
      "wheatfield_medium",
      "wheatfield_ripe",
      "wheatfield_harvested",
      "atlanteans_resi_none",
      "atlanteans_resi_water",
      "atlanteans_resi_coal_1",
      "atlanteans_resi_iron_1",
      "atlanteans_resi_gold_1",
      "atlanteans_resi_stones_1",
      "atlanteans_resi_coal_2",
      "atlanteans_resi_iron_2",
      "atlanteans_resi_gold_2",
      "atlanteans_resi_stones_2",
      "atlanteans_shipconstruction",
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      "europeans_headquarters",
      "europeans_warehouse",
      "europeans_port",

      -- Small
      "europeans_well_basic",
      "europeans_well",
      "europeans_well_advanced",
      "europeans_quarry_basic",
      "europeans_quarry_marble",
      "europeans_quarry_quartz",
      "europeans_claydiggers_house",
      "europeans_lumberjacks_hut",
      "europeans_woodcutters_house",
      "europeans_woodcutters_house_advanced",
      "europeans_rangers_hut",
      "europeans_foresters_house",
      "europeans_foresters_house_advanced",
      "europeans_fishers_hut",
      "europeans_fishers_house",
      "europeans_fishers_house_advanced",
      "europeans_hunters_hut",
      "europeans_hunters_house",
      "europeans_hunters_house_advanced",
      "europeans_beekeepers_hut",
      "europeans_beekeepers_house",
      "europeans_scouts_house",

      -- Medium
      "europeans_reed_yard",
      "europeans_stonemasons_hut",
      "europeans_stonemasons_house",
      "europeans_farm_medium",
      "europeans_mill_basic",
      "europeans_mill",
      "europeans_mill_advanced",
      "europeans_bakery_basic",
      "europeans_bakery",
      "europeans_bakery_advanced",
      "europeans_smokery_basic",
      "europeans_smokery",
      "europeans_smokery_advanced",
      "europeans_farm_fruit",
      "europeans_brewery_basic",
      "europeans_brewery",
      "europeans_winery",
      "europeans_sawmill_basic",
      "europeans_sawmill",
      "europeans_sawmill_advanced",
      "europeans_charcoal_kiln_basic",
      "europeans_charcoal_kiln",
      "europeans_charcoal_kiln_advanced",
      "europeans_smelting_works_basic",
      "europeans_smelting_works",
      "europeans_smelting_works_advanced",
      "europeans_toolsmithy",
      "europeans_armorsmithy",
      "europeans_weaponsmithy",
      "europeans_barracks",
      "europeans_shipyard",
      "europeans_ferry_yard",

      -- Big
      "europeans_farm_basic",
      "europeans_farm_big",
      "europeans_farm_advanced",
      "europeans_spiderfarm",
      "europeans_sheepfarm",
      "europeans_weaving_mill_basic",
      "europeans_weaving_mill",
      "europeans_weaving_mill_advanced",

      -- Mines
      "europeans_coalmine_basic",
      "europeans_coalmine",
      "europeans_coalmine_deep",
      "europeans_ironmine_basic",
      "europeans_ironmine",
      "europeans_ironmine_deep",
      "europeans_goldmine_basic",
      "europeans_goldmine",
      "europeans_goldmine_deep",

      -- Training Sites
      "europeans_guardhall",
      "europeans_dungeon",
      "europeans_trainingscamp_sword",
      "europeans_trainingscamp_trident",
      "europeans_trainingscamp_spear",
      "europeans_trainingscamp_ax",
      "europeans_colosseum",
      "europeans_arena",
      "europeans_labyrinth",
      "europeans_trainingscamp_big",

      -- Military Sites
      "europeans_sentry",
      "europeans_blockhouse",
      "europeans_tower_small",
      "europeans_guardhouse",
      "europeans_advanced_tower",
      "europeans_tower_high",
      "europeans_tower",
      "europeans_advanced_castle",
      "europeans_fortress",
      "europeans_castle",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      "Åland",
      "Azoren",
      "Britania",
      "Corsica",
      "Creta",
      "Cyprus",
      "El Hierro",
      "Elba",
      "Färöer",
      "Fuerteventura",
      "Gibraltar",
      "Gran Canaria",
      "Grönland",
      "Guernsey",
      "Ibiza",
      "Ireland",
      "Isle of Man",
      "Island",
      "Jersey",
      "La Gomera",
      "La Palma",
      "Lanzarote",
      "Lesbos",
      "Madeira",
      "Mallorca",
      "Malta",
      "Menorca",
      "Rhodos",
      "Sardenya",
      "Scandinavia",
      "Sizilia",
      "Teneriffa",
   },
   ferry_names = {
      "Albania",
      "Andorra",
      "Austria",
      "Belaruz",
      "Belgium",
      "Bosnia",
      "Bulgaria",
      "Croatia",
      "Czechia",
      "Danmark",
      "England",
      "Espana",
      "Estnia",
      "France",
      "Germania",
      "Hellas",
      "Helvetia",
      "Herzegovina",
      "Hungaria",
      "Italia",
      "Kosovo",
      "Latvia",
      "Lituva",
      "Liechtenstein",
      "Mazedonia",
      "Moldavia",
      "Monaco",
      "Montenegro",
      "Nederlands",
      "Norge",
      "Poland",
      "Portuguesa",
      "Romania",
      "Russia",
      "San Marino",
      "Serbia",
      "Slowakia",
      "Slowenia",
      "Scotia",
      "Suomi",
      "Sverige",
      "Ukrajina",
      "Vatikan",
      "Wales",
   },

   -- Special types
   builder = "europeans_builder",
   carrier = "europeans_carrier_1",
   carrier2 = "europeans_carrier_2",
   geologist = "europeans_geologist",
   soldier = "europeans_soldier",
   ship = "europeans_ship",
   ferry = "europeans_ferry",
   port = "europeans_port",
}
