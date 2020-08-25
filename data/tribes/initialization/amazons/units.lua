-- TODO(GunChleoc): Remove compatibility/deprecation info after v1.0
-- This is the documentation for the init.lua file

-- RST
-- .. _lua_tribes_tribes_init:
--
-- Bootstrapping a Tribe
-- ---------------------
--
-- Each tribe needs to have some basic information defined for displaying it in menus
-- and for loading its game starting conditions.
-- The file **has to be located in** ``tribes/initialization/<tribename>/init.lua`` so that the engine can find it.
--
-- This file returns a table with the basic information for a tribe that is
-- needed before a game is loaded or the editor has been started. The table has the following entries:
--
-- * **name**: A string containing the internal name of the tribe
-- * **author**: The creator(s) of the tribe
-- * **descname**: In-game display name of the tribe
-- * **tooltip**: A description of the tribe to be shown in tooltips
-- * **icon**: File path to a png file to be used as button image
-- * **script**: File path to the :ref:`units.lua <lua_tribes_tribes_units>` file
--   that configures which units the tribe is using
-- * **starting_conditions**: A table of file paths to starting condition Lua scripts
--

-- And now the documentation for this file

-- RST
-- .. _lua_tribes_tribes_units:
--
-- Configuring a Tribe's Units Usage
-- ---------------------------------
--
-- The ``tribes/initialization/<tribename>/units.lua`` file configures the units that a tribe can use.
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
--    **animations**: Global animations. Contains subtables for ``frontier`` and ``flag``.
--    Each animation needs the parameter ``hotspot`` (2 integer coordinates),
--    and may also define ``fps`` (integer frames per second).
--
--    **animation_directory**: The location of the animation png files.
--
--    **bridges**: Contains animations for ``normal_e``, ``normal_se``, ``normal_sw``,
--    ``busy_e``, ``busy_se`` and ``busy_sw``.
--
--    **bridge_height**: The height in pixels of each bridge at it's summit at 1x scale.
--
--    **roads**: The file paths for the tribe's road textures in 3 subtables ``busy``,
--    ``normal`` and ``waterway``.
--
--    **resource_indicators**: The names for the resource indicators.
--    This table contains a subtable for each resource name plus a subtable named
--    ``""`` for no resources. Each subtable is an array, in which the index of
--    each entry is the highest amount of resources the indicator may indicate.
--
--    **wares_order**: This defines all the wares that this tribe uses and their
--    display order in the user interface. Each subtable defines a column in the
--    user interface, and in a current development version it contains subtables
--    in turn for each ware referenced. The subtables define the ware's ``name``,
--    ``default_target_quantity`` and ``preciousness``, like this:
--
--    .. code-block:: lua
--
--       wares_order = {
--          {
--             -- Building Materials
--             {
--                name = "granite",
--                default_target_quantity = 20,
--                preciousness = 5
--             },
--             {
--               ...
--
--    The meaning of the ware parameters is as follows:
--
--       **name**: A string containing the internal name of this ware.
--
--       **default_target_quantity**: *Optional* The default target quantity
--       for the tribe's economy.
--       If not set, the economy will always demand this ware.
--       If set to zero, the economy will not demand this ware unless it is required
--       in a production building.
--       If not set or set to zero, the actual target quantity will not be available
--       in the economy settings window.
--
--       **preciousness**: How precious this ware is to this tribe.
--       We recommend not going higher than ``50``.
--
--    **workers_order**:  This defines all the workers that this tribe uses and their
--    display order in the user interface. Each subtable defines a column in the user
--    interface, and in a current development version it contains subtables
--    in turn for each worker referenced. The subtables define the worker's ``name``,
--    ``default_target_quantity`` and ``preciousness``, like this:
--
--    .. code-block:: lua
--
--       workers_order = {
--          {
--             -- Carriers
--             { name = "amazons_carrier" },
--             { name = "amazons_ferry" },
--             {
--                name = "amazons_tapir",
--                default_target_quantity = 10,
--                preciousness = 2
--             },
--             { name = "amazons_tapir_breeder" }
--          },
--          {
--               ...
--
--    The meaning of the worker parameters is the same as for the wares, but
--    both ``default_target_quantity`` and ``preciousness`` are optional.
--    However, when ``default_target_quantity`` has been set, you will also need
--    to set ``preciousness``.
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
--          left_corner = dirname .. "images/amazons/toolbar_left_corner.png",
--          left = dirname .. "images/amazons/toolbar_left.png", -- Will be tiled
--          center = dirname .. "images/amazons/toolbar_center.png",
--          right = dirname .. "images/amazons/toolbar_right.png", -- Will be tiled
--          right_corner = dirname .. "images/amazons/toolbar_right_corner.png"
--       }
--

tribes = wl.Tribes()
image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes")

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
         image_dirname .. "waterway_0.png",
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
         {
            name = "log",
            preciousness = 24
         },
         {
            name = "granite",
            default_target_quantity = 20,
            preciousness = 7
         },
         {
            name = "balsa",
            default_target_quantity = 20,
            preciousness = 20
         },
         {
            name = "ironwood",
            default_target_quantity = 40,
            preciousness = 50
         },
         {
            name = "rubber",
            default_target_quantity = 30,
            preciousness = 40
         },
         {
            name = "liana",
            preciousness = 8
         },
         {
            name = "rope",
            preciousness = 2
         },
      },
      {
         -- Food
         {
            name = "water",
            preciousness = 15
         },
         {
            name = "cassavaroot",
            preciousness = 10
         },
         {
            name = "cocoa_beans",
            preciousness = 1
         },
         {
            name = "amazons_bread",
            default_target_quantity = 20,
            preciousness = 5
         },
         {
            name = "chocolate",
            default_target_quantity = 15,
            preciousness = 5
         },
         {
            name = "fish",
            preciousness = 4
         },
         {
            name = "meat",
            preciousness = 2
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 5
         },
      },
      {
         -- Mining
         {
            name = "gold_dust",
            default_target_quantity = 15,
            preciousness = 2
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2
         },
         {
            name = "charcoal",
            default_target_quantity = 20,
            preciousness = 10
         },
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 3,
            preciousness = 1
         },
         {
            name = "felling_ax",
            default_target_quantity = 5,
            preciousness = 3
         },
         {
            name = "shovel",
            default_target_quantity = 2,
            preciousness = 0
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1
         },
         {
            name = "machete",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "spear_wooden",
            default_target_quantity = 30,
            preciousness = 3
         },
         {
            name = "chisel",
            default_target_quantity = 2,
            preciousness = 1
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "needles",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "stonebowl",
            default_target_quantity = 1,
            preciousness = 1
         },
      },
      {
         -- Military
         {
            name = "spear_stone_tipped",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "spear_hardened",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "armor_wooden",
            default_target_quantity = 1,
            preciousness = 0
         },
         {
            name = "helmet_wooden",
            default_target_quantity = 2,
            preciousness = 3
         },
         {
            name = "warriors_coat",
            default_target_quantity = 1,
            preciousness = 3
         },
         {
            name = "tunic",
            default_target_quantity = 30,
            preciousness = 3
         },
         {
            name = "vest_padded",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "protector_padded",
            default_target_quantity = 1,
            preciousness = 1
         },
         {
            name = "boots_sturdy",
            default_target_quantity = 1,
            preciousness = 5
         },
         {
            name = "boots_swift",
            default_target_quantity = 1,
            preciousness = 5
         },
                  {
            name = "boots_hero",
            default_target_quantity = 1,
            preciousness = 5
         },
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         { name = "amazons_carrier" },
         { name = "amazons_ferry" },
         {
            name = "amazons_tapir",
            default_target_quantity = 10,
            preciousness = 2
         },
         { name = "amazons_tapir_breeder" },
      },
      {
         -- Building Materials
         { name = "amazons_stonecutter" },
         { name = "amazons_woodcutter" },
         { name = "amazons_woodcutter_master" },
         { name = "amazons_jungle_preserver" },
         { name = "amazons_jungle_master" },
         { name = "amazons_liana_cutter" },
         { name = "amazons_builder" },
         { name = "amazons_shipwright" },
      },
      {
         -- Food
         { name = "amazons_hunter_gatherer" },
         { name = "amazons_cassava_farmer" },
         { name = "amazons_cocoa_farmer" },
         { name = "amazons_cook" },
         { name = "amazons_wilderness_keeper" },
      },
      {
         -- Mining
         { name = "amazons_geologist" },
         { name = "amazons_gold_digger" },
         { name = "amazons_charcoal_burner" },
         { name = "amazons_gold_smelter" },
      },
      {
         -- Tools
         { name = "amazons_stonecarver" }
      },
      {
         -- Military
         {
            name = "amazons_soldier",
            default_target_quantity = 10,
            preciousness = 5
         },
         { name = "amazons_trainer" },
         { name = "amazons_dressmaker" },
         { name = "amazons_scout" }
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
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Orinoco"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Amazonas"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Abacaxis"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Anchicaya"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Guaitara"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Iscuande"),
      -- TRANSLATORS: This Amazonian ship is named after a river
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

   toolbar = {
      left_corner = image_dirname .. "toolbar_left_corner.png",
      left = image_dirname .. "toolbar_left.png",
      center = image_dirname .. "toolbar_center.png",
      right = image_dirname .. "toolbar_right.png",
      right_corner = image_dirname .. "toolbar_right_corner.png"
   }

}

pop_textdomain()
