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
-- * **collectors_points_table**: An array of tables of warename-integer pairs used by the Collectors win condition.
-- * **suited_for_ai** (optional, defaults to ``true``): Whether it is allowed to assign this tribe to AI players.
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
--    **military_capacity_script**: File path to the :ref:`military_capacity.lua <lua_tribes_tribes_military_capacity>` file.
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
--                name = "atlanteans_carrier",
--                helptexts = {
--                   purpose = pgettext("atlanteans_worker", "Carries items along your roads.")
--                }
--             },
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
--             name = "atlanteans_shipyard",
--             helptexts = {
--                purpose = pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
--             }
--          },
--       }
--
--    **builder**:  The internal name of the tribe's builder. This unit needs to be defined in the ``workers_order`` table too.
--
--    **carriers**:  An :class:`array` with the internal names of the tribe's carrier and beasts of burden. Must have at least two entries,
--                   the first of which is used as the tribe's primary carrier and the second of which as the tribe's secondary carrier.
--                   These units need to be defined in the ``workers_order`` table too.
--
--    **carrier**:  **Deprecated**. Replaced by **carriers**.
--
--    **carrier2**:  **Deprecated**. Replaced by **carriers**.
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
--    **scouts_house**:  The internal name of the tribe's scout's hut or house. This unit needs to be defined in the ``buildings`` table too.
--
--    **toolbar**: *Optional*. Replace the default toolbar images with these custom images. Example:
--
--    **productionsite_worker_missing**: The string to display over this tribe's productionsites when 1 worker is missing.
--
--    **productionsite_worker_coming**: The string to display over this tribe's productionsites when 1 worker is coming.
--
--    **productionsite_workers_missing**: The string to display over this tribe's productionsites when more than 1 worker is missing.
--
--    **productionsite_workers_coming**: The string to display over this tribe's productionsites when more than 1 worker is coming.
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
-- * The tribe’s scouting building should additionally define a ``no_scouting_building_connected``
--   text which will be shown in the UI when the Send Scout To Flag button is disabled for lack
--   of a connected scout’s house/hut.
-- * We recommend that you use ``pgettext`` to disambiguate the strings for the different tribes.
-- * To make life easier for our translators, you can split long helptexts into multiple entries
--   as with the ``performance`` example above.
--   The helptexts are then joined by the engine.
--   In our example, we will get *"If all needed wares are delivered in time, this building can produce each type of ax in about 57 seconds on average. All three weapons take the same time for making, but the required raw materials vary."*

descriptions = wl.Descriptions()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

descriptions:new_tribe {
   name = "atlanteans",
   military_capacity_script = path.dirname(__file__) .. "military_capacity.lua",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {3, 12} },
      bridge_normal_e = { hotspot = {-2, 11} },
      bridge_busy_e = { hotspot = {-2, 11} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 6} },
      bridge_busy_sw = { hotspot = {36, 6} }
   },
   spritesheets = {
      flag = {
         fps = 10,
         frames = 15,
         rows = 5,
         columns = 3,
         hotspot = { 11, 35 }
      },
   },

   bridge_height = 8,

   collectors_points_table = {
      { ware = "gold", points = 3},
      { ware = "trident_light", points = 2},
      { ware = "trident_long", points = 3},
      { ware = "trident_steel", points = 4},
      { ware = "trident_double", points = 7},
      { ware = "trident_heavy_double", points = 8},
      { ware = "shield_steel", points = 4},
      { ware = "shield_advanced", points = 7},
      { ware = "tabard", points = 1},
      { ware = "tabard_golden", points = 5},
   },

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
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Granite, part 2
                  pgettext("atlanteans_ware", "The Atlanteans produce granite blocks in quarries and crystal mines.")
               }
            }
         },
         {
            name = "log",
            preciousness = 14,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Log, part 1
                  pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Log, part 2
                  pgettext("atlanteans_ware", "Atlanteans use logs also as the base for planks, which are used in nearly every building. Besides the sawmill, the charcoal kiln, the toolsmithy and the smokery also need logs for their work.")
               }
            }
         },
         {
            name = "planks",
            default_target_quantity = 40,
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 2
                  pgettext("atlanteans_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Planks, part 3
                  pgettext("atlanteans_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
               }
            }
         },
         {
            name = "spider_silk",
            default_target_quantity = 10,
            preciousness = 11,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Spider Silk
               purpose = pgettext("atlanteans_ware", "Spider silk is produced by spiders, which are bred by spider farms. It is processed into spidercloth in a weaving mill.")
            }
         },
         {
            name = "spidercloth",
            default_target_quantity = 20,
            preciousness = 7,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Spidercloth
               purpose = pgettext("atlanteans_ware", "Spidercloth is made out of spider silk in a weaving mill. It is used in the toolsmithy and the shipyard. Also some higher developed buildings need spidercloth for their construction.")
            }
         },
      },
      {
         -- Food
         {
            name = "fish",
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Fish
               purpose = pgettext("atlanteans_ware", "Fish is one of the biggest food resources of the Atlanteans. It has to be smoked in a smokery before being delivered to mines, training sites and scouts.")
            }
         },
         {
            name = "smoked_fish",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Smoked Fish
               purpose = pgettext("atlanteans_ware", "As no Atlantean likes raw fish, smoking it in a smokery is the most common way to make it edible.")
            }
         },
         {
            name = "meat",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Meat, part 1
                  pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Meat, part 2
                  pgettext("atlanteans_ware", "Meat has to be smoked in a smokery before being delivered to mines and training sites (dungeon and labyrinth).")
               }
            }
         },
         {
            name = "smoked_meat",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Smoked Meat
               purpose = pgettext("atlanteans_ware", "Smoked meat is made out of meat in a smokery. It is delivered to the mines and training sites (labyrinth and dungeon) where the miners and soldiers prepare a nutritious lunch for themselves.")
            }
         },
         {
            name = "water",
            preciousness = 7,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for an atlantean ware: Water, part 2
                  pgettext("atlanteans_ware", "Water is used in the bakery and the horse and spider farms.")
               }
            }
         },
         {
            name = "corn",
            preciousness = 12,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Corn
               purpose = pgettext("atlanteans_ware", "This corn is processed in the mill into fine cornmeal that every Atlantean baker needs for a good bread. Also horse and spider farms need to be provided with corn.")
            }
         },
         {
            name = "cornmeal",
            default_target_quantity = 15,
            preciousness = 7,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Cornmeal
               purpose = pgettext("atlanteans_ware", "Cornmeal is produced in a mill out of corn and is one of three parts of the Atlantean bread produced in bakeries.")
            }
         },
         {
            name = "blackroot",
            preciousness = 10,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Blackroot
               purpose = pgettext("atlanteans_ware", "Blackroots are a special kind of root produced at blackroot farms and processed in mills. The Atlanteans like their strong taste and use their flour for making bread.")
            }
         },
         {
            name = "blackroot_flour",
            default_target_quantity = 0,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Blackroot Flour
               purpose = pgettext("atlanteans_ware", "Blackroot Flour is produced in mills out of blackroots. It is used in bakeries to make a tasty bread.")
            }
         },
         {
            name = "atlanteans_bread",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Bread
               purpose = pgettext("atlanteans_ware", "This tasty bread is made in bakeries out of cornmeal, blackroot flour and water. It is appreciated as basic food by miners, scouts and soldiers in training sites (labyrinth and dungeon).")
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
               -- TRANSLATORS: Helptext for an atlantean ware: Quartz
               purpose = pgettext("atlanteans_ware", "These transparent quartz gems are used to build some exclusive buildings. They are produced in a crystal mine.")
            }
         },
         {
            name = "diamond",
            default_target_quantity = 5,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Diamond
               purpose = pgettext("atlanteans_ware", "These wonderful diamonds are used to build some exclusive buildings. They are mined in a crystal mine.")
            }
         },
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Coal, part 1
                  pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Coal, part 2
                  pgettext("atlanteans_ware", "The Atlantean fires in smelting works, armor smithies and weapon smithies are fed with coal.")
               }
            }
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Iron Ore, part 1
                  pgettext("default_ware", "Iron ore is mined in iron mines."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Iron Ore, part 2
                  pgettext("atlanteans_ware", "It is smelted in a smelting works to retrieve the iron.")
               }
            }
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Iron, part 1
                  pgettext("ware", "Iron is smelted out of iron ores."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Iron, part 2
                  pgettext("atlanteans_ware", "It is produced by the smelting works and used in the toolsmithy, armor smithy and weapon smithy.")
               }
            }
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Gold Ore, part 1
                  pgettext("ware", "Gold ore is mined in a gold mine."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Gold Ore, part 2
                  pgettext("atlanteans_ware", "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
               }
            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Gold, part 1
                  pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Gold, part 2
                  pgettext("atlanteans_ware", "It is produced by the smelting works and used by the armor smithy, the weapon smithy and the gold spinning mill.")
               }
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
               -- TRANSLATORS: Helptext for an atlantean ware: Pick
               purpose = pgettext("ware", "Picks are used by stonecutters and miners. They are produced by the toolsmith.")
            }
         },
         {
            name = "saw",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Saw
               purpose = pgettext("atlanteans_ware", "The saw is needed by the sawyer, the woodcutter and the toolsmith. It is produced by the toolsmith.")
            }
         },
         {
            name = "shovel",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Shovel, part 1
                  pgettext("ware", "Shovels are needed for the proper handling of plants."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Shovel, part 2
                  pgettext("atlanteans_ware", "Therefore the forester and the blackroot farmer use them. They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Hammer, part 2
                  pgettext("atlanteans_ware", "Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "milking_tongs",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Milking Tongs
               purpose = pgettext("atlanteans_ware", "Milking tongs are used by the spider breeder to milk the spiders. They are produced by the toolsmith.")
            }
         },
         {
            name = "fishing_net",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Fishing Net
               purpose = pgettext("atlanteans_ware", "The fishing net is used by the fisher and produced by the toolsmith.")
            }
         },
         {
            name = "buckets",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Buckets
               purpose = pgettext("atlanteans_ware", "Big buckets for the fish breeder – produced by the toolsmith.")
            }
         },
         {
            name = "hunting_bow",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Hunting Bow
               purpose = pgettext("atlanteans_ware", "This bow is used by the Atlantean hunter. It is produced by the toolsmith.")
            }
         },
         {
            name = "hook_pole",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Hook Pole
               purpose = pgettext("atlanteans_ware", "This hook pole is used by the smoker to suspend all the meat and fish from the top of the smokery. It is created by the toolsmith.")
            }
         },
         {
            name = "scythe",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Scythe, part 1
                  pgettext("ware", "The scythe is the tool of the farmers."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Scythe, part 2
                  pgettext("atlanteans_ware", "Scythes are produced by the toolsmith.")
               }
            }
         },
         {
            name = "bread_paddle",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Bread Paddle, part 1
                  pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Bread Paddle, part 2
                  pgettext("atlanteans_ware", "Bread paddles are produced by the toolsmith.")
               }
            }
         },
         {
            name = "fire_tongs",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an atlantean ware: Fire Tongs, part 1
                  pgettext("ware", "Fire tongs are the tools for smelting ores."),
                  -- TRANSLATORS: Helptext for an atlantean ware: Fire Tongs, part 2
                  pgettext("atlanteans_ware_fire_tongs", "They are used in the smelting works and produced by the toolsmith.")
               }
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
               -- TRANSLATORS: Helptext for an atlantean ware: Light Trident
               purpose = pgettext("atlanteans_ware", "This is the basic weapon of the Atlantean soldiers. Together with a tabard, it makes up the equipment of young soldiers. Light tridents are produced in the weapon smithy as are all other tridents.")
            }
         },
         {
            name = "trident_long",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Long Trident
               purpose = pgettext("atlanteans_ware", "The long trident is the first trident in the training of soldiers. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 0 to level 1.")
            }
         },
         {
            name = "trident_steel",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Steel Trident
               purpose = pgettext("atlanteans_ware", "This is the medium trident. It is produced in the weapon smithy and used by advanced soldiers in the dungeon – together with food – to train from attack level 1 to level 2.")
            }
         },
         {
            name = "trident_double",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Double Trident
               purpose = pgettext("atlanteans_ware", "The double trident is one of the best tridents produced by the Atlantean weapon smithy. It is used in a dungeon – together with food – to train soldiers from attack level 2 to level 3.")
            }
         },
         {
            name = "trident_heavy_double",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Heavy Double Trident
               purpose = pgettext("atlanteans_ware", "This is the most dangerous weapon of the Atlantean military. Only the best of the best soldiers may use it. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 3 to level 4.")
            }
         },
         {
            name = "shield_steel",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Steel Shield
               purpose = pgettext("atlanteans_ware", "This steel shield is produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 0 to level 1.")
            }
         },
         {
            name = "shield_advanced",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Advanced Shield
               purpose = pgettext("atlanteans_ware", "These advanced shields are used by the best soldiers of the Atlanteans. They are produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 1 to level 2.")
            }
         },
         {
            name = "tabard",
            default_target_quantity = 30,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Tabard
               purpose = pgettext("atlanteans_ware", "A tabard and a light trident are the basic equipment for young soldiers. Tabards are produced in the weaving mill.")
            }
         },
         {
            name = "gold_thread",
            default_target_quantity = 5,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Gold Thread
               purpose = pgettext("atlanteans_ware", "This thread, made of gold by the gold spinning mill, is used for weaving the exclusive golden tabard in the weaving mill.")
            }
         },
         {
            name = "tabard_golden",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Golden Tabard
               purpose = pgettext("atlanteans_ware", "Golden tabards are produced in Atlantean weaving mills out of gold thread. They are used in the labyrinth – together with food – to train soldiers from health level 0 to level 1.")
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
               -- TRANSLATORS: Helptext for an atlantean worker: Carrier
               purpose = pgettext("atlanteans_worker", "Carries items along your roads.")
            }
         },
         {
            name = "atlanteans_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Ferry
               purpose = pgettext("atlanteans_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "atlanteans_horse",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Horse
               purpose = pgettext("atlanteans_worker", "Horses help to carry items along busy roads. They are reared in a horse farm.")
            }
         },
         {
            name = "atlanteans_horsebreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Horse Breeder
               purpose = pgettext("atlanteans_worker", "Breeds the strong Atlantean horses for adding them to the transportation system.")
            }
         }
      },
      {
         -- Building Materials
         {
            name = "atlanteans_stonecutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Stonecutter
               purpose = pgettext("atlanteans_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "atlanteans_woodcutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Woodcutter
               purpose = pgettext("atlanteans_worker", "Fells trees.")
            }
         },
         {
            name = "atlanteans_sawyer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Sawyer
               purpose = pgettext("atlanteans_worker", "Saws logs to produce planks.")
            }
         },
         {
            name = "atlanteans_forester",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Forester
               purpose = pgettext("atlanteans_worker", "Plants trees.")
            }
         },
         {
            name = "atlanteans_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Builder
               purpose = pgettext("atlanteans_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "atlanteans_spiderbreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Spider Breeder
               purpose = pgettext("atlanteans_worker", "Breeds spiders for silk.")
            }
         },
         {
            name = "atlanteans_weaver",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Weaver
               purpose = pgettext("atlanteans_worker", "Produces spidercloth for buildings, ships and soldiers.")
            }
         },
         {
            name = "atlanteans_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Shipwright
               purpose = pgettext("atlanteans_worker", "Works at the shipyard and constructs new ships.")
            }
         }
      },
      {
         -- Food
         {
            name = "atlanteans_fisher",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Fisher
               purpose = pgettext("atlanteans_worker", "The fisher fishes delicious fish.")
            }
         },
         {
            name = "atlanteans_fishbreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Fish Breeder
               purpose = pgettext("atlanteans_worker", "Breeds fish.")
            }
         },
         {
            name = "atlanteans_hunter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Hunter
               purpose = pgettext("atlanteans_worker", "The hunter brings fresh, raw meat to the colonists.")
            }
         },
         {
            name = "atlanteans_smoker",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Smoker
               purpose = pgettext("atlanteans_worker", "Smokes meat and fish.")
            }
         },
         {
            name = "atlanteans_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Farmer
               purpose = pgettext("atlanteans_worker", "Plants and harvests cornfields.")
            }
         },
         {
            name = "atlanteans_blackroot_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Blackroot Farmer
               purpose = pgettext("atlanteans_worker", "Plants and harvests blackroot.")
            }
         },
         {
            name = "atlanteans_miller",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Miller
               purpose = pgettext("atlanteans_worker", "Grinds blackroots and corn to produce blackroot flour and cornmeal, respectively.")
            }
         },
         {
            name = "atlanteans_baker",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Baker
               purpose = pgettext("atlanteans_worker", "Bakes bread for workers.")
            }
         }
      },
      {
         -- Mining
         {
            name = "atlanteans_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Geologist
               purpose = pgettext("atlanteans_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "atlanteans_miner",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Miner
               purpose = pgettext("atlanteans_worker", "Works deep in the mines to obtain coal, iron, gold or precious stones.")
            }
         },
         {
            name = "atlanteans_charcoal_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Charcoal Burner
               purpose = pgettext("atlanteans_worker", "Burns coal.")
            }
         },
         {
            name = "atlanteans_smelter",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Smelter
               purpose = pgettext("atlanteans_worker", "Smelts ores into metal.")
            }
         }
      },
      {
         -- Tools
         {
            name = "atlanteans_toolsmith",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Toolsmith
               purpose = pgettext("atlanteans_worker", "Produces tools for the workers.")
            }
         }
      },
      {
         -- Military
         {
            name = "atlanteans_recruit",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Recruit
               purpose = pgettext("atlanteans_worker", "Eager to become a soldier and defend his tribe!")
            }
         },
         {
            name = "atlanteans_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Soldier
               purpose = pgettext("atlanteans_worker", "Defend and Conquer!")
            }
         },
         {
            name = "atlanteans_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Trainer
               purpose = pgettext("atlanteans_worker", "Trains the soldiers.")
            }
         },
         {
            name = "atlanteans_weaponsmith",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Weaponsmith
               purpose = pgettext("atlanteans_worker", "Produces weapons for the soldiers.")
            }
         },
         {
            name = "atlanteans_armorsmith",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Armorsmith
               purpose = pgettext("atlanteans_worker", "Produces armor for the soldiers.")
            }
         },
         {
            name = "atlanteans_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean worker: Scout
               purpose = pgettext("atlanteans_worker", "Scouts like Scotty the scout scouting unscouted areas in a scouty fashion.")
               -- (c) WiHack Team 02.01.2010
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
         name = "blackrootfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "blackrootfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "blackrootfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "blackrootfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "blackrootfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Blackroot Field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "cornfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "cornfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cornfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cornfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "cornfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Corn Field
            purpose = _("This field has been harvested.")
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
         name = "atlanteans_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "atlanteans_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean resource indicator: Water
            purpose = _("There is water in the ground here that can be pulled up by a well.")
         }
      },
      {
         name = "atlanteans_resi_coal_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Coal, part 2
               _("There is only a little bit of coal here.")
            }
         }
      },
      {
         name = "atlanteans_resi_iron_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Iron, part 2
               _("There is only a little bit of iron here.")
            }
         }
      },
      {
         name = "atlanteans_resi_gold_1",
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
         name = "atlanteans_resi_stones_1",
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
         name = "atlanteans_resi_coal_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Coal, part 2
               _("There is a lot of coal here.")
            }
         }
      },
      {
         name = "atlanteans_resi_iron_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for an atlantean resource indicator: Iron, part 2
               _("There is a lot of iron here.")
            }
         }
      },
      {
         name = "atlanteans_resi_gold_2",
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
         name = "atlanteans_resi_stones_2",
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
         name = "atlanteans_shipconstruction",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Ship Under Construction
            purpose = _("A ship is being constructed at this site.")
         }
      },
      -- non atlantean Immovables used by the woodcutter
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Dead Tree
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by atlanteans: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "atlanteans_headquarters",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Headquarters
            purpose = pgettext("atlanteans_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for an atlantean warehouse: Headquarters
            note = pgettext("atlanteans_building", "The headquarters is your main building.")
         }
      },
      {
         name = "atlanteans_warehouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Warehouse
            purpose = pgettext("atlanteans_building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "atlanteans_port",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Port
            purpose = pgettext("atlanteans_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools."),
            -- TRANSLATORS: Note helptext for an atlantean warehouse: Port
            note = pgettext("atlanteans_building", "Similar to the Headquarters a Port can be attacked and destroyed by an enemy. It is recommendable to send soldiers to defend it.")
         }
      },

      -- Small
      {
         name = "atlanteans_quarry",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Quarry
            purpose = pgettext("atlanteans_building", "Cuts blocks of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Quarry
            note = pgettext("atlanteans_building", "The quarry needs rocks to cut within the work area.")
         }
      },
      {
         name = "atlanteans_woodcutters_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Woodcutter's House
            purpose = pgettext("building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Woodcutter's House
            note = pgettext("atlanteans_building", "The woodcutter’s house needs trees to fell within the work area.")
         }
      },
      {
         name = "atlanteans_foresters_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Forester's House
            purpose = pgettext("building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Forester's House
            note = pgettext("atlanteans_building", "The forester’s house needs free space within the work area to plant the trees.")
         }
      },
      {
         name = "atlanteans_fishers_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Fisher's House
            purpose = pgettext("atlanteans_building", "Fishes on the coast near the fisher’s house."),
            note = {
               -- TRANSLATORS: Note helptext for an atlantean production site: Fisher's House, part 1
               pgettext("atlanteans_building", "The fisher’s house needs water full of fish within the work area."),
               -- TRANSLATORS: Note helptext for an atlantean production site: Fisher's House, part 2
               pgettext("atlanteans_building", "Build a fish breeder’s house close to the fisher’s house to make sure that you don’t run out of fish.")
            }
         }
      },
      {
         name = "atlanteans_fishbreeders_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an atlantean production site: Fish Breeder's House
            lore = pgettext("atlanteans_building", [[‘Only after the last duck has been shot down<br>]] ..
                                          [[Only after the last deer has been put to death<br>]] ..
                                          [[Only after the last fish has been caught<br>]] ..
                                          [[Then will you find that spiders are not to be eaten.’]]),
            -- TRANSLATORS: Lore author helptext for an atlantean production site: Fish Breeder's House
            lore_author = pgettext("atlanteans_building", "Prophecy of the fish breeders"),
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Fish Breeder's House
            purpose = pgettext("building", "Breeds fish.")
         }
      },
      {
         name = "atlanteans_hunters_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Hunter's House
            purpose = pgettext("building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Hunter's House
            note = pgettext("atlanteans_building", "The hunter’s house needs animals to hunt within the work area.")
         }
      },
      {
         name = "atlanteans_well",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Well
            purpose = pgettext("building", "Draws water out of the deep.")
         }
      },
      {
         name = "atlanteans_gold_spinning_mill",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Gold Spinning Mill
            purpose = pgettext("atlanteans_building", "Spins gold thread out of gold.")
         }
      },
      {
         name = "atlanteans_scouts_house",
         helptexts = {
            no_scouting_building_connected = pgettext("atlanteans_building", "You need to connect this flag to a scout’s house before you can send a scout here."),
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Scout's House
            purpose = pgettext("building", "Explores unknown territory.")
         }
      },

      -- Medium
      {
         name = "atlanteans_sawmill",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Sawmill
            purpose = pgettext("building", "Saws logs to produce planks.")
         }
      },
      {
         name = "atlanteans_smokery",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Smokery
            purpose = pgettext("atlanteans_building", "Smokes meat and fish to feed the scouts and miners and to train soldiers in the dungeon.")
         }
      },
      {
         name = "atlanteans_mill",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Mill
            purpose = pgettext("atlanteans_building", "Grinds blackroots and corn to produce blackroot flour and cornmeal, respectively."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Mill
            note = pgettext("atlanteans_building", "When no cornmeal is required, the mill will try to produce blackroot flour even when there is no demand for it.")
         }
      },
      {
         name = "atlanteans_bakery",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Bakery
            purpose = pgettext("atlanteans_building", "Bakes bread to feed the scouts and miners and to train soldiers.")
         }
      },
      {
         name = "atlanteans_charcoal_kiln",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Charcoal Kiln
            purpose = pgettext("building", "Burns logs into charcoal.")
         }
      },
      {
         name = "atlanteans_smelting_works",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Smelting Works
            purpose = pgettext("building", "Smelts iron ore into iron and gold ore into gold.")
         }
      },
      {
         name = "atlanteans_toolsmithy",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("building", "Forges all the tools that your workers need.")
         }
      },
      {
         name = "atlanteans_weaponsmithy",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Weapon Smithy
            purpose = pgettext("atlanteans_building", "Forges tridents to equip the soldiers and to train their attack in the dungeon.")
         }
      },
      {
         name = "atlanteans_armorsmithy",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Armor Smithy
            purpose = pgettext("atlanteans_building", "Forges shields that are used for training soldiers’ defense in the labyrinth.")
         }
      },
      {
         name = "atlanteans_barracks",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Barracks
            purpose = pgettext("atlanteans_building", "Equips recruits and trains them as soldiers.")
         }
      },

      -- Big
      {
         name = "atlanteans_horsefarm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Horse Farm
            purpose = pgettext("atlanteans_building", "Breeds the strong Atlantean horses for adding them to the transportation system.")
         }
      },
      {
         name = "atlanteans_farm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Farm
            purpose = pgettext("atlanteans_building", "Sows and harvests corn."),
            -- TRANSLATORS: Performance helptext for an atlantean production site: Farm
            performance = pgettext("atlanteans_building", "The farmer needs %1% on average to sow and harvest a sheaf of corn."):bformat(format_minutes_seconds(1, 20))
         }
      },
      {
         name = "atlanteans_blackroot_farm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Blackroot Farm
            purpose = pgettext("atlanteans_building", "Sows and harvests blackroot."),
            -- TRANSLATORS: Performance helptext for an atlantean production site: Blackroot Farm
            performance = pgettext("atlanteans_building", "The blackroot farmer needs %1% on average to sow and harvest a bundle of blackroot."):bformat(format_minutes_seconds(1, 20))
         }
      },
      {
         name = "atlanteans_spiderfarm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Spider Farm
            purpose = pgettext("building", "Breeds spiders for silk.")
         }
      },
      {
         name = "atlanteans_weaving_mill",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Weaving Mill
            purpose = pgettext("atlanteans_building", "Weaves spidercloth for buildings and ships’ sails, and tabards to equip and train the soldiers.")
         }
      },

      -- Mines
      {
         name = "atlanteans_crystalmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Crystal Mine
            purpose = pgettext("atlanteans_building", "Carves precious stones out of the rock in mountain terrain.")
         }
      },
      {
         name = "atlanteans_coalmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "atlanteans_ironmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "atlanteans_goldmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },

      -- Training Sites
      {
         name = "atlanteans_dungeon",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Dungeon, part 1
               pgettext("atlanteans_building", "Trains soldiers in ‘Attack’."),
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Dungeon, part 2
               pgettext("atlanteans_building", "Equips the soldiers with all necessary weapons and armor parts.")
            }
         }
      },
      {
         name = "atlanteans_labyrinth",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Labyrinth, part 1
               pgettext("atlanteans_building", "Trains soldiers in ‘Defense’, ‘Evade’, and ‘Health’."),
               -- TRANSLATORS: Purpose helptext for an atlantean training site: Labyrinth, part 2
               pgettext("atlanteans_building", "Equips the soldiers with all necessary weapons and armor parts.")
            }
         }
      },

      -- Military Sites
      {
         name = "atlanteans_guardhouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Guardhouse
            purpose = pgettext("atlanteans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Guardhouse
            note = pgettext("atlanteans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "atlanteans_guardhall",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Guardhall
            purpose = pgettext("atlanteans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Guardhall
            note = pgettext("atlanteans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "atlanteans_tower_small",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Small Tower
            purpose = pgettext("atlanteans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Small Tower
            note = pgettext("atlanteans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "atlanteans_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Tower
            purpose = pgettext("atlanteans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Tower
            note = pgettext("atlanteans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "atlanteans_tower_high",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: High Tower
            purpose = pgettext("atlanteans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: High Tower
            note = pgettext("atlanteans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "atlanteans_castle",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Castle
            purpose = pgettext("atlanteans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an atlantean military site: Castle
            note = pgettext("atlanteans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "atlanteans_ferry_yard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Ferry Yard
            purpose = pgettext("building", "Builds ferries."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Ferry Yard
            note = pgettext("building", "Needs water nearby.")
         }
      },
      {
         name = "atlanteans_shipyard",
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

   -- Productionsite status strings

   -- TRANSLATORS: Productivity label on an atlantean building if there is 1 worker missing
   productionsite_worker_missing = pgettext("atlanteans", "Worker missing"),
   -- TRANSLATORS: Productivity label on an atlantean building if there is 1 worker coming
   productionsite_worker_coming = pgettext("atlanteans", "Worker is coming"),
   -- TRANSLATORS: Productivity label on an atlantean building if there is more than 1 worker missing. If you need plural forms here, please let us know.
   productionsite_workers_missing = pgettext("atlanteans", "Workers missing"),
   -- TRANSLATORS: Productivity label on an atlantean building if there is more than 1 worker coming. If you need plural forms here, please let us know.
   productionsite_workers_coming = pgettext("atlanteans", "Workers are coming"),

   -- Special types
   builder = "atlanteans_builder",
   carriers = {"atlanteans_carrier", "atlanteans_horse"},
   geologist = "atlanteans_geologist",
   scouts_house = "atlanteans_scouts_house",
   soldier = "atlanteans_soldier",
   ship = "atlanteans_ship",
   ferry = "atlanteans_ferry",
   port = "atlanteans_port",
}

pop_textdomain()
