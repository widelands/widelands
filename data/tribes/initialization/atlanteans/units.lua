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
--             { name = "atlanteans_carrier" },
--             { name = "atlanteans_ferry" },
--             {
--                name = "atlanteans_horse",
--                default_target_quantity = 10,
--                preciousness = 2
--             },
--             { name = "atlanteans_horsebreeder" }
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

-- NOCOM documentation
tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

tribes:new_tribe {
   name = "atlanteans",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {3, 12} },
      flag = { hotspot = {15, 35}, fps = 10 },
      bridge_normal_e = { hotspot = {-2, 11} },
      bridge_busy_e = { hotspot = {-2, 11} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 6} },
      bridge_busy_sw = { hotspot = {36, 6} }
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
      resource_coal = {
         [10] = "atlanteans_resi_coal_1",
         [20] = "atlanteans_resi_coal_2",
      },
      resource_iron = {
         [10] = "atlanteans_resi_iron_1",
         [20] = "atlanteans_resi_iron_2",
      },
      resource_gold = {
         [10] = "atlanteans_resi_gold_1",
         [20] = "atlanteans_resi_gold_2",
      },
      resource_stones = {
         [10] = "atlanteans_resi_stones_1",
         [20] = "atlanteans_resi_stones_2",
      },
      resource_water = {
         [100] = "atlanteans_resi_water",
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
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "log",
            preciousness = 14,
            helptexts = {

            }
         },
         {
            name = "planks",
            default_target_quantity = 40,
            preciousness = 10,
            helptexts = {

            }
         },
         {
            name = "spider_silk",
            default_target_quantity = 10,
            preciousness = 11,
            helptexts = {

            }
         },
         {
            name = "spidercloth",
            default_target_quantity = 20,
            preciousness = 7,
            helptexts = {

            }
         },
      },
      {
         -- Food
         {
            name = "fish",
            preciousness = 4,
            helptexts = {

            }
         },
         {
            name = "smoked_fish",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "meat",
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "smoked_meat",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "water",
            preciousness = 7,
            helptexts = {

            }
         },
         {
            name = "corn",
            preciousness = 12,
            helptexts = {

            }
         },
         {
            name = "cornmeal",
            default_target_quantity = 15,
            preciousness = 7,
            helptexts = {

            }
         },
         {
            name = "blackroot",
            preciousness = 10,
            helptexts = {

            }
         },
         {
            name = "blackroot_flour",
            default_target_quantity = 0,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "atlanteans_bread",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {

            }
         }
      },
      {
         -- Mining
         {
            name = "quartz",
            default_target_quantity = 5,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "diamond",
            default_target_quantity = 5,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {

            }
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 4,
            helptexts = {

            }
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {

            }
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {

            }
         }
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 3,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "saw",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "shovel",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "milking_tongs",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "fishing_net",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "buckets",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "hunting_bow",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "hook_pole",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "scythe",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "bread_paddle",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "fire_tongs",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         }
      },
      {
         -- Military
         {
            name = "trident_light",
            default_target_quantity = 30,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "trident_long",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "trident_steel",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "trident_double",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "trident_heavy_double",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "shield_steel",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "shield_advanced",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "tabard",
            default_target_quantity = 30,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "gold_thread",
            default_target_quantity = 5,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "tabard_golden",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         }
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         {
            name = "atlanteans_carrier",
            helptexts = {

            }
         },
         {
            name = "atlanteans_ferry",
            helptexts = {

            }
         },
         {
            name = "atlanteans_horse",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "atlanteans_horsebreeder",
            helptexts = {

            }
         }
      },
      {
         -- Building Materials
         {
            name = "atlanteans_stonecutter",
            helptexts = {

            }
         },
         {
            name = "atlanteans_woodcutter",
            helptexts = {

            }
         },
         {
            name = "atlanteans_sawyer",
            helptexts = {

            }
         },
         {
            name = "atlanteans_forester",
            helptexts = {

            }
         },
         {
            name = "atlanteans_builder",
            helptexts = {

            }
         },
         {
            name = "atlanteans_spiderbreeder",
            helptexts = {

            }
         },
         {
            name = "atlanteans_weaver",
            helptexts = {

            }
         },
         {
            name = "atlanteans_shipwright",
            helptexts = {

            }
         }
      },
      {
         -- Food
         {
            name = "atlanteans_fisher",
            helptexts = {

            }
         },
         {
            name = "atlanteans_fishbreeder",
            helptexts = {

            }
         },
         {
            name = "atlanteans_hunter",
            helptexts = {

            }
         },
         {
            name = "atlanteans_smoker",
            helptexts = {

            }
         },
         {
            name = "atlanteans_farmer",
            helptexts = {

            }
         },
         {
            name = "atlanteans_blackroot_farmer",
            helptexts = {

            }
         },
         {
            name = "atlanteans_miller",
            helptexts = {

            }
         },
         {
            name = "atlanteans_baker",
            helptexts = {

            }
         }
      },
      {
         -- Mining
         {
            name = "atlanteans_geologist",
            helptexts = {

            }
         },
         {
            name = "atlanteans_miner",
            helptexts = {

            }
         },
         {
            name = "atlanteans_charcoal_burner",
            helptexts = {

            }
         },
         {
            name = "atlanteans_smelter",
            helptexts = {

            }
         }
      },
      {
         -- Tools
         {
            name = "atlanteans_toolsmith",
            helptexts = {

            }
         }
      },
      {
         -- Military
         {
            name = "atlanteans_recruit",
            helptexts = {

            }
         },
         {
            name = "atlanteans_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "atlanteans_trainer",
            helptexts = {

            }
         },
         {
            name = "atlanteans_weaponsmith",
            helptexts = {

            }
         },
         {
            name = "atlanteans_armorsmith",
            helptexts = {

            }
         },
         {
            name = "atlanteans_scout",
            helptexts = {

            }
         }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {

         }
      },
      {
         name = "blackrootfield_tiny",
         helptexts = {

         }
      },
      {
         name = "blackrootfield_small",
         helptexts = {

         }
      },
      {
         name = "blackrootfield_medium",
         helptexts = {

         }
      },
      {
         name = "blackrootfield_ripe",
         helptexts = {

         }
      },
      {
         name = "blackrootfield_harvested",
         helptexts = {

         }
      },
      {
         name = "cornfield_tiny",
         helptexts = {

         }
      },
      {
         name = "cornfield_small",
         helptexts = {

         }
      },
      {
         name = "cornfield_medium",
         helptexts = {

         }
      },
      {
         name = "cornfield_ripe",
         helptexts = {

         }
      },
      {
         name = "cornfield_harvested",
         helptexts = {

         }
      },
      {
         name = "destroyed_building",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_none",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_water",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_coal_1",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_iron_1",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_gold_1",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_stones_1",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_coal_2",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_iron_2",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_gold_2",
         helptexts = {

         }
      },
      {
         name = "atlanteans_resi_stones_2",
         helptexts = {

         }
      },
      {
         name = "atlanteans_shipconstruction",
         helptexts = {

         }
      }
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "atlanteans_headquarters",
         helptexts = {

         }
      },
      {
         name = "atlanteans_warehouse",
         helptexts = {

         }
      },
      {
         name = "atlanteans_port",
         helptexts = {

         }
      },

      -- Small
      {
         name = "atlanteans_quarry",
         helptexts = {

         }
      },
      {
         name = "atlanteans_woodcutters_house",
         helptexts = {

         }
      },
      {
         name = "atlanteans_foresters_house",
         helptexts = {

         }
      },
      {
         name = "atlanteans_fishers_house",
         helptexts = {

         }
      },
      {
         name = "atlanteans_fishbreeders_house",
         helptexts = {

         }
      },
      {
         name = "atlanteans_hunters_house",
         helptexts = {

         }
      },
      {
         name = "atlanteans_well",
         helptexts = {

         }
      },
      {
         name = "atlanteans_gold_spinning_mill",
         helptexts = {

         }
      },
      {
         name = "atlanteans_scouts_house",
         helptexts = {

         }
      },

      -- Medium
      {
         name = "atlanteans_sawmill",
         helptexts = {

         }
      },
      {
         name = "atlanteans_smokery",
         helptexts = {

         }
      },
      {
         name = "atlanteans_mill",
         helptexts = {

         }
      },
      {
         name = "atlanteans_bakery",
         helptexts = {

         }
      },
      {
         name = "atlanteans_charcoal_kiln",
         helptexts = {

         }
      },
      {
         name = "atlanteans_smelting_works",
         helptexts = {

         }
      },
      {
         name = "atlanteans_toolsmithy",
         helptexts = {

         }
      },
      {
         name = "atlanteans_weaponsmithy",
         helptexts = {

         }
      },
      {
         name = "atlanteans_armorsmithy",
         helptexts = {

         }
      },
      {
         name = "atlanteans_barracks",
         helptexts = {

         }
      },

      -- Big
      {
         name = "atlanteans_horsefarm",
         helptexts = {

         }
      },
      {
         name = "atlanteans_farm",
         helptexts = {

         }
      },
      {
         name = "atlanteans_blackroot_farm",
         helptexts = {

         }
      },
      {
         name = "atlanteans_spiderfarm",
         helptexts = {

         }
      },
      {
         name = "atlanteans_weaving_mill",
         helptexts = {

         }
      },

      -- Mines
      {
         name = "atlanteans_crystalmine",
         helptexts = {

         }
      },
      {
         name = "atlanteans_coalmine",
         helptexts = {

         }
      },
      {
         name = "atlanteans_ironmine",
         helptexts = {

         }
      },
      {
         name = "atlanteans_goldmine",
         helptexts = {

         }
      },

      -- Training Sites
      {
         name = "atlanteans_dungeon",
         helptexts = {

         }
      },
      {
         name = "atlanteans_labyrinth",
         helptexts = {

         }
      },

      -- Military Sites
      {
         name = "atlanteans_guardhouse",
         helptexts = {

         }
      },
      {
         name = "atlanteans_guardhall",
         helptexts = {

         }
      },
      {
         name = "atlanteans_tower_small",
         helptexts = {

         }
      },
      {
         name = "atlanteans_tower",
         helptexts = {

         }
      },
      {
         name = "atlanteans_tower_high",
         helptexts = {

         }
      },
      {
         name = "atlanteans_castle",
         helptexts = {

         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "atlanteans_ferry_yard",
         helptexts = {

         }
      },
      {
         name = "atlanteans_shipyard",
         helptexts = {

         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {

         }
      },
      {
         name = "dismantlesite",
         helptexts = {

         }
      }
   },

   -- Special types
   builder = "atlanteans_builder",
   carrier = "atlanteans_carrier",
   carrier2 = "atlanteans_horse",
   geologist = "atlanteans_geologist",
   soldier = "atlanteans_soldier",
   ship = "atlanteans_ship",
   ferry = "atlanteans_ferry",
   port = "atlanteans_port",

   toolbar = {
      left_corner = image_dirname .. "toolbar_left_corner.png",
      left = image_dirname .. "toolbar_left.png",
      center = image_dirname .. "toolbar_center.png",
      right = image_dirname .. "toolbar_right.png",
      right_corner = image_dirname .. "toolbar_right_corner.png"
   }
}

pop_textdomain()
