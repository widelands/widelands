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
--    ``default_target_quantity``, ``preciousness``, and :ref:`lua_tribes_tribes_helptexts`,
--    like this:
--
--    .. code-block:: lua
--
--       wares_order = {
--          {
--             -- Building Materials
--             {
--                name = "granite",
--                default_target_quantity = 20,
--                preciousness = 5,
--                helptexts = {
--                   purpose = pgettext("ware", "Granite is a basic building material.")
--                }
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
--    ``default_target_quantity``, ``preciousness`` and :ref:`lua_tribes_tribes_helptexts`,
--    like this:
--
--    .. code-block:: lua
--
--       workers_order = {
--          {
--             -- Carriers
--             {
--                name = "amazons_carrier",
--                helptexts = {
--                   purpose = pgettext("amazons_worker", "Carries items along your roads.")
--                }
--             },
--             { name = "amazons_ferry" },
--             {
--                name = "amazons_tapir",
--                default_target_quantity = 10,
--                preciousness = 2
--             },
--             { name = "amazons_horsebreeder" }
--          },
--          {
--               ...
--
--    The meaning of the worker parameters is the same as for the wares, but
--    both ``default_target_quantity`` and ``preciousness`` are optional.
--    However, when ``default_target_quantity`` has been set, you will also need
--    to set ``preciousness``.
--
--    **immovables**: This defines the name and :ref:`lua_tribes_tribes_helptexts`
--    for all the immovables that this tribe uses, like this:
--
--    .. code-block:: lua
--
--       immovables = {
--          {
--             name = "ashes",
--             helptexts = {
--                purpose = _("The remains of a destroyed building.")
--             }
--          },
--          {
--             ...
--       }
--
--    **buildings**: This defines the name and :ref:`lua_tribes_tribes_helptexts`
--    for all the buildings that this tribe uses and their display order in the user interface, like this:
--
--    .. code-block:: lua
--
--       buildings = {
--          {
--             name = "amazons_shipyard",
--             helptexts = {
--                purpose = pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
--             }
--          },
--       }
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
--
-- .. _lua_tribes_tribes_helptexts:
--
-- Helptexts
-- ---------
--
-- Helptexts are used in the Tribal Encyclopedia to give the users some basic
-- information and lore about units.
-- They are optional and defined in a ``helptexts`` subtable in the unit's listing.
--
-- Example for a building:
--
-- .. code-block:: lua
--
--    buildings = {
--       {
--          name = "barbarians_ax_workshop",
--          helptexts = {
--             -- Lore helptext for a barbarian production site: Ax Workshop
--             lore = pgettext("barbarians_building", "‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’"),
--
--             -- Lore author helptext for a barbarian production site: Ax Workshop
--             lore_author = pgettext("barbarians_building", "An old Barbarian proverb<br> meaning that you need to take some risks sometimes."),
--
--             -- Purpose helptext for a barbarian production site: Ax Workshop
--             purpose = pgettext("barbarians_building", "Produces axes, sharp axes and broad axes."),
--
--             -- Note helptext for a barbarian production site: Ax Workshop
--             note = pgettext("barbarians_building", "The Barbarian ax workshop is the intermediate production site in a series of three buildings. It is enhanced from the metal workshop but doesn’t require additional qualification for the worker."),
--
--             performance = {
--                -- Performance helptext for a barbarian production site: Ax Workshop, part 1
--                pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57)),
--                -- Performance helptext for a barbarian production site: Ax Workshop, part 2
--                pgettext("barbarians_building", "All three weapons take the same time for making, but the required raw materials vary.")
--             }
--          },
--          ...
--       },
--       immovables {
--          ...
--       },
--       wares {
--          ...
--       },
--       workers {
--          ...
--       }
--    }
--
-- * All units should have a ``purpose`` helptext, but this is not enforced by the engine.
-- * Empty helptexts are allowed, although they will log a warning to the console to
--   help you find missing helptexts.
-- * ``lore``, ``lore_author`` and ``note`` are only used by buildings,
--   ``performance`` is only used by training site and production site buildings.
-- * We recommend that you use ``pgettext`` to disambiguate the strings for the different tribes.
-- * To make life easier for our translators, you can split long helptexts into multiple entries
--   as with the ``performance`` example above.
--   The helptexts are then joined by the engine.
--   In our example, we will get *"If all needed wares are delivered in time, this building can produce each type of ax in about 57 seconds on average. All three weapons take the same time for making, but the required raw materials vary."*

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
      resource_coal = {
         [100] = "amazons_resi_none",
      },
      resource_iron = {
         [100] = "amazons_resi_none",
      },
      resource_gold = {
         [10] = "amazons_resi_gold_1",
         [20] = "amazons_resi_gold_2",
      },
      resource_stones = {
         [10] = "amazons_resi_stones_1",
         [20] = "amazons_resi_stones_2",
      },
      resource_water = {
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
            preciousness = 24,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "granite",
            default_target_quantity = 20,
            preciousness = 7,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "balsa",
            default_target_quantity = 20,
            preciousness = 20,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "ironwood",
            default_target_quantity = 40,
            preciousness = 50,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "rubber",
            default_target_quantity = 30,
            preciousness = 40,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "liana",
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "rope",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
      },
      {
         -- Food
         {
            name = "water",
            preciousness = 15,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "cassavaroot",
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "cocoa_beans",
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "amazons_bread",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "chocolate",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "fish",
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "meat",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
      },
      {
         -- Mining
         {
            name = "gold_dust",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "charcoal",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 3,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 5,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "shovel",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "machete",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "spear_wooden",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "chisel",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "needles",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "stonebowl",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
      },
      {
         -- Military
         {
            name = "spear_stone_tipped",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "spear_hardened",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "armor_wooden",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "helmet_wooden",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "warriors_coat",
            default_target_quantity = 1,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "tunic",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "vest_padded",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "protector_padded",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "boots_sturdy",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "boots_swift",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
                  {
            name = "boots_hero",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("amazons_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("amazons_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         {
            name = "amazons_carrier",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Carrier
               purpose = pgettext("amazons_worker", "Carries items along your roads.")
            }
         },
         {
            name = "amazons_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Ferry
               purpose = pgettext("amazons_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "amazons_tapir",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Horse
               purpose = pgettext("amazons_worker", "tapirs help to carry items along busy roads. They are reared in a horse farm.")
            }
         },
         {
            name = "amazons_tapirbreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Horse Breeder
               purpose = pgettext("amazons_worker", "Breeds the tapirs for adding them to the transportation system.")
            }
         }
      },
      {
         -- Building Materials
         {
            name = "amazons_stonecutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_woodcutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_woodcutter_master",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_jungle_preserver",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_jungle_master",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_liana_cutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
      },
      {
         -- Food
         {
            name = "amazons_hunter_gatherer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_cassava_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_cocoa_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_cook",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_wilderness_keeper",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
      },
      {
         -- Mining
         {
            name = "amazons_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_gold_digger",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_charcoal_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_gold_smelter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
      },
      {
         -- Tools
         {
            name = "amazons_stonecarver",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         }
      },
      {
         -- Military
         {
            name = "amazons_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_dressmaker",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Ashes
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "destroyed_building",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Destroyed Building
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Ashes
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "cassavarootfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "cassavarootfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cassavarootfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cassavarootfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "cassavarootfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "cocoa_tree_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "cocoa_tree_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "cocoa_tree_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "cocoa_tree_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "amazons_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "amazons_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean resource indicator: Water
            purpose = _("There is water in the ground here that can be pulled up by a well.")
         }
      },
      {
         name = "amazons_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "amazons_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones, part 1
               _("Precious stones are used in the construction of big buildings. They can be dug up by a crystal mine. You will also get granite from the mine."),
               -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones, part 2
               _("There are only a few precious stones here.")
            }
         }
      },
      {
         name = "amazons_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "amazons_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones, part 1
               _("Precious stones are used in the construction of big buildings. They can be dug up by a crystal mine. You will also get granite from the mine."),
               -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones, part 2
               _("There are many precious stones here.")
            }
         }
      },
      {
         name = "amazons_shipconstruction",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Ship Under Construction
            purpose = _("A ship is being constructed at this site.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "amazons_headquarters",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Headquarters
            purpose = pgettext("amazons_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for an atlantean warehouse: Headquarters
            note = pgettext("amazons_building", "The headquarters is your main building.")
         }
      },
      {
         name = "amazons_warehouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Warehouse
            purpose = pgettext("building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "amazons_port",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Port
            purpose = pgettext("amazons_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools.")
         }
      },


      -- Small
      {
         name = "amazons_stonecutters_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Quarry
            purpose = pgettext("amazons_building", "Cuts blocks of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Quarry
            note = pgettext("amazons_building", "The quarry needs rocks to cut within the work area.")
         }
      },
      {
         name = "amazons_woodcutters_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Woodcutter's House
            purpose = pgettext("building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Woodcutter's House
            note = pgettext("amazons_building", "The woodcutter’s house needs trees to fell within the work area.")
         }
      },
      {
         name = "amazons_junglepreservers_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Forester's House
            purpose = pgettext("building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Forester's House
            note = pgettext("amazons_building", "The forester’s house needs free space within the work area to plant the trees.")
         }
      },
      {
         name = "amazons_hunter_gatherers_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Fisher's House
            purpose = pgettext("amazons_building", "Fishes on the coast near the fisher’s house."),
            note = {
               -- TRANSLATORS: Note helptext for an atlantean production site: Fisher's House, part 1
               pgettext("amazons_building", "The fisher’s house needs water full of fish within the work area."),
               -- TRANSLATORS: Note helptext for an atlantean production site: Fisher's House, part 2
               pgettext("amazons_building", "Build a fish breeder’s house close to the fisher’s house to make sure that you don’t run out of fish.")
            }
         }
      },
      {
         name = "amazons_liana_cutters_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Fish Breeder's House
            purpose = pgettext("building", "Breeds fish.")
         }
      },
      {
         name = "amazons_water_gatherers_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Hunter's House
            purpose = pgettext("building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Hunter's House
            note = pgettext("amazons_building", "The hunter’s house needs animals to hunt within the work area.")
         }
      },
      {
         name = "amazons_rare_trees_woodcutters_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Well
            purpose = pgettext("building", "Draws water out of the deep.")
         }
      },
      {
         name = "amazons_wilderness_keepers_tent",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Gold Spinning Mill
            purpose = pgettext("amazons_building", "Spins gold thread out of gold.")
         }
      },
      {
         name = "amazons_scouts_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Scout's House
            purpose = pgettext("building", "Explores unknown territory.")
         }
      },

      -- Medium
      {
         name = "amazons_rope_weaver_booth",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Sawmill
            purpose = pgettext("building", "Saws logs to produce planks.")
         }
      },
      {
         name = "amazons_furnace",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Smokery
            purpose = pgettext("amazons_building", "Smokes meat and fish to feed the scouts and miners and to train soldiers in the dungeon.")
         }
      },
      {
         name = "amazons_rare_tree_plantation",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Mill
            purpose = pgettext("amazons_building", "Grinds blackroots and corn to produce blackroot flour and cornmeal, respectively."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Mill
            note = pgettext("amazons_building", "When no cornmeal is required, the mill will try to produce blackroot flour even when there is no demand for it.")
         }
      },
      {
         name = "amazons_stonecarvery",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Bakery
            purpose = pgettext("amazons_building", "Bakes bread to feed the scouts and miners and to train soldiers.")
         }
      },
      {
         name = "amazons_dressmakery",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Charcoal Kiln
            purpose = pgettext("building", "Burns logs into charcoal.")
         }
      },
      {
         name = "amazons_charcoal_kiln",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Smelting Works
            purpose = pgettext("building", "Smelts iron ore into iron and gold ore into gold.")
         }
      },
      {
         name = "amazons_cassava_root_cooker",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("building", "Forges all the tools that your workers need.")
         }
      },
      {
         name = "amazons_chocolate_brewery",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Weapon Smithy
            purpose = pgettext("amazons_building", "Forges tridents to equip the soldiers and to train their attack in the dungeon.")
         }
      },
      {
         name = "amazons_food_preserver",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Armor Smithy
            purpose = pgettext("amazons_building", "Forges shields that are used for training soldiers’ defense in the labyrinth.")
         }
      },
      {
         name = "amazons_youth_gathering",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Barracks
            purpose = pgettext("amazons_building", "Equips recruits and trains them as soldiers.")
         }
      },
      {
         name = "amazons_gardening_center",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Barracks
            purpose = pgettext("amazons_building", "Equips recruits and trains them as soldiers.")
         }
      },

      -- Big
      {
         name = "amazons_tapir_farm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Horse Farm
            purpose = pgettext("amazons_building", "Breeds the strong Atlantean horses for adding them to the transportation system.")
         }
      },
      {
         name = "amazons_cassava_root_plantation",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Farm
            purpose = pgettext("amazons_building", "Sows and harvests corn."),
            -- TRANSLATORS: Performance helptext for an atlantean production site: Farm
            performance = pgettext("amazons_building", "The farmer needs %1% on average to sow and harvest a sheaf of corn."):bformat(format_minutes_seconds(1, 20))
         }
      },
      {
         name = "amazons_cocoa_farm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Blackroot Farm
            purpose = pgettext("amazons_building", "Sows and harvests blackroot."),
            -- TRANSLATORS: Performance helptext for an atlantean production site: Blackroot Farm
            performance = pgettext("amazons_building", "The blackroot farmer needs %1% on average to sow and harvest a bundle of blackroot."):bformat(format_minutes_seconds(1, 20))
         }
      },
      "",
      "",
      "",

      -- Mines
      {
         name = "amazons_stonemine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Crystal Mine
            purpose = pgettext("amazons_building", "Carves precious stones out of the rock in mountain terrain.")
         }
      },
      {
         name = "amazons_gold_digger_dwelling",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain.")
         }
      },

      -- Training Sites
      {
         name = "amazons_warriors_gathering",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Dungeon, part 1
               pgettext("amazons_building", "Trains soldiers in ‘Attack’."),
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Dungeon, part 2
               pgettext("amazons_building", "Equips the soldiers with all necessary weapons and armor parts.")
            }
         }
      },
      {
         name = "amazons_training_glade",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Labyrinth, part 1
               pgettext("amazons_building", "Trains soldiers in ‘Defense’, ‘Evade’, and ‘Health’."),
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Labyrinth, part 2
               pgettext("amazons_building", "Equips the soldiers with all necessary weapons and armor parts.")
            }
         }
      },

      -- Military Sites
      {
         name = "amazons_patrol_post",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Guardhouse
            purpose = pgettext("building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Guardhouse
            note = pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "amazons_treetop_sentry",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Guardhall
            purpose = pgettext("building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Guardhall
            note = pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "amazons_warriors_dwelling",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Small Tower
            purpose = pgettext("building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Small Tower
            note = pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "amazons_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Tower
            purpose = pgettext("building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Tower
            note = pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "amazons_observation_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: High Tower
            purpose = pgettext("building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: High Tower
            note = pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "amazons_fortress",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Castle
            purpose = pgettext("building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Castle
            note = pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "amazons_fortification",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Castle
            purpose = pgettext("building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Castle
            note = pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "amazons_ferry_yard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Ferry Yard
            purpose = pgettext("building", "Builds ferries."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Ferry Yard
            note = pgettext("building", "Needs water nearby.")
         }
      },
      {
         name = "amazons_shipyard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Shipyard
            purpose = pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an atlantean building: Construction Site
            lore = pgettext("building", "‘Don’t swear at the builder who is short of building materials.’"),
            -- TRANSLATORS: Lore author helptext for an atlantean building: Construction Site
            lore_author = pgettext("building", "Proverb widely used for impossible tasks of any kind"),
            -- TRANSLATORS: Purpose helptext for an atlantean building: Construction Site
            purpose = pgettext("building", "A new building is being built at this construction site.")
         }
      },
      {
         name = "dismantlesite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an atlantean building: Dismantle Site
            lore = pgettext("building", "‘New paths will appear when you are willing to tear down the old.’"),
            -- TRANSLATORS: Lore author helptext for an atlantean building: Dismantle Site
            lore_author = pgettext("building", "Proverb"),
            -- TRANSLATORS: Purpose helptext for an atlantean building: Dismantle Site
            purpose = pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      }
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
