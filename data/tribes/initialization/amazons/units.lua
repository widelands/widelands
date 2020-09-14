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

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

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
                  -- TRANSLATORS: Helptext for an amazon ware: Log, part 1
                  pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
                  -- TRANSLATORS: Helptext for an amazon ware: Log, part 2
                  pgettext("amazons_ware", "Amazon woodcutters fell trees and jungle preservers are planting them. They are used by the stonecarver. The charcoal kiln also needs logs to produce charcoal.")
               }
            }
         },
         {
            name = "granite",
            default_target_quantity = 20,
            preciousness = 7,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for an amazon ware: Granite, part 2
                  pgettext("amazons_ware", "The Amazons use granite for making tools and weapons as well. It is produced by the stonecutters and in stone mines."),
               }
            }
         },
         {
            name = "balsa",
            default_target_quantity = 20,
            preciousness = 20,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: balsa
                  pgettext("amazons_ware", "Balsa wood is an important building material.It is also used to make light armor. Balsa trees are cut by an experienced woodcutter.")
               }
            }
         },
         {
            name = "ironwood",
            default_target_quantity = 40,
            preciousness = 50,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Ironwood
                  pgettext("amazons_ware", "This very hard wood is as hard as iron and it is used for several buildings, tools and weapons. It is cut by a very experienced woodcutter.")
               }
            }
         },
         {
            name = "rubber",
            default_target_quantity = 30,
            preciousness = 40,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Rubber
                  pgettext("amazons_ware", "Rubber is important for making warriors armor and shoes. Rubber trees are collected from experienced woodcutters.")
               }
            }
         },
         {
            name = "liana",
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Liana
                  pgettext("amazons_ware", "Lianas grow in the trees. Very durable ropes are made out of it.")
               }
            }
         },
         {
            name = "rope",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Rope
                  pgettext("amazons_ware", "This rope is made out of liana. It is used for armor dresses and to construct buildings and ships.")
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
                  -- TRANSLATORS: Helptext for an amazon ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for an amazon ware: Water, part 2
                  pgettext("amazons_ware", "Water is used by the cassava root cookery, the chocolate brewery, the gold digger and the tapir farms.")
               }
            }
         },
         {
            name = "cassavaroot",
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Cassavaroot
                  pgettext("amazons_ware", "Cassavaroots are a special kind of root produced at cassava farms. The Amazons like their strong taste for making bread and they feed also tapirs with it.")
               }
            }
         },
         {
            name = "cocoa_beans",
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Cocoa Beans
                  pgettext("amazons_ware", "Cocoa beans are gathered from cocoa bushes by a cocoa farmer. They are used for producing chocolate.")
               }
            }
         },
         {
            name = "amazons_bread",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Bread
                  pgettext("amazons_ware", "This tasty bread is made in cookeries out of cassava root and water. It is the basic ingredient of a ration that is made by a food preserver. Soldiers like it too.")
               }
            }
         },
         {
            name = "chocolate",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Chocolate
                  pgettext("amazons_ware", "Chocolate is produced by chocolate breweries and used to train soldiers.")
               }
            }
         },
         {
            name = "fish",
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Fish
                  pgettext("amazons_ware", "Fish is one of the two major food resources of the Amazons. It is used by the food preserver to prepare rations for the miners. It is also consumed at the training sites (training glade and warriors gathering).")
               }
            }
         },
         {
            name = "meat",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Meat, part 1
                  pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
                  -- TRANSLATORS: Helptext for an amazon ware: Meat, part 2
                  pgettext("amazons_ware", "Meat is used by the food preserver to prepare rations for the miners. It is also consumed at the training sites (training glade and warriors gathering).")
               }
            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Ration, part 1
                  pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
                  -- TRANSLATORS: Helptext for an amazon ware: Ration, part 2
                  pgettext("amazons_ware", "Rations are produced by the food preserver out of fish or meat and bread. They are dried by charcoal.")
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
                  -- TRANSLATORS: Helptext for an amazon ware: Gold Dust, part 1
                  pgettext("ware", "Gold dust is washed in a gold digger dwelling."),
                  -- TRANSLATORS: Helptext for an amazon ware: Gold Dust, part 2
                  pgettext("amazons_ware", "Smelted in a furnace, it turns into gold which is used as a precious building material and to produce armor.")
               }
            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Gold, part 1
                  pgettext("ware", "Gold is the most valuable of all metals,"),
                  -- TRANSLATORS: Helptext for an amazon ware: Gold, part 2
                  pgettext("amazons_ware", "and it is smelted out of gold dust. It is produced by the furnace and used by the dressmaker and for special buildings.")
               }
            }
         },
         {
            name = "charcoal",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Charcoal, part 1
                  pgettext("amazons_ware", "Charcoal produced out of logs by a charcoal burner. The Amazons use charcoal for drying their food and smelting gold.")
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
                  -- TRANSLATORS: Helptext for an amazon ware: Pick
                  pgettext("amazons_ware", "Picks are used by stonecutters to cut granite from rocks or in mines.")
               }
            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 5,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Felling Ax, part 1
                  pgettext("ware", "The felling ax is the tool to chop down trees."),
                  -- TRANSLATORS: Helptext for an amazon ware: Felling Ax, part 2
                  pgettext("amazons_ware", "They are used by the woodcutters and produced by the stonecarver.")
               }
            }
         },
         {
            name = "shovel",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Shovel, part 1
                  pgettext("ware", "Shovels are needed for the proper handling of plants."),
                  -- TRANSLATORS: Helptext for an amazon ware: Shovel, part 2
                  pgettext("amazons_ware", "Therefore the jungle preservers and the cassava and cocoa farmers use them. They are produced by the stonecarver.")
               }
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for an amazon ware: Hammer, part 2
                  pgettext("amazons_ware", "Geologists, builders, shipwrights and stonecarvers all need a hammer. Make sure you’ve always got some in reserve! They are produced by the stonecarver.")
               }
            }
         },
         {
            name = "machete",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Machete
                  pgettext("ware", "The machete is the tool of the liana cutters."),
               }
            }
         },
         {
            name = "spear_wooden",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Wooden Spear, part 1
                  pgettext("amazons_ware", "This spear is the basic weapon in the Amazon military system. It is produced by the stonecarver. In combination with a tunic, it is the equipment to fit out young soldiers."),
                  -- TRANSLATORS: Helptext for an amazon ware: Wooden Spear, part 2
                  pgettext("amazons_ware", "It is used by the amazon hunters as well.")
               }
            }
         },
         {
            name = "chisel",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Chisel, part 1
                  pgettext("ware", "The chisel is an essential tool."),
                  -- TRANSLATORS: Helptext for an amazon ware: Chisel, part 2
                  pgettext("amazons_ware", "Stonecarvers need a chisel. Make sure you’ve always got some in reserve! They are produced by the stonecarver itself.")
               }
            }
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Kitchen Tools
                  pgettext("amazons_ware", "Kitchen tools are needed for preparing rations, baking cassava bread and brewing chocolate. Be sure to have a stonecarver to produce this basic tool.")
               }
            }
         },
         {
            name = "needles",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Needles
                  pgettext("amazons_ware", "Needles are used by dressmakers to make dresses, boots and armor for the soldiers.")
               }
            }
         },
         {
            name = "stonebowl",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Stone Bowl
                  pgettext("amazons_ware", "Stonebowls are used to wash gold and to smelt the gold dust.")
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
                  -- TRANSLATORS: Helptext for an amazon ware: Stone tipped Spear
                  pgettext("amazons_ware", "This spear has a stone spike. It is produced in a stone carvery and used in a warriors gathering – together with food – to train soldiers from attack level 0 to attack level 1.")
               }
            }
         },
         {
            name = "spear_hardened",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Hardened Spear
                  pgettext("amazons_ware", "This is an hardened spear with a stone tip. It is produced in a stone carvery and used in a warriors gathering – together with food – to train soldiers from attack level 1 to level 2.")
               }
            }
         },
         {
            name = "armor_wooden",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Wooden Armor
                  pgettext("amazons_ware", "The light wooden armor is a basic armor for amazons soldiers. It is produced by a dressmaker and used in a training glade – together with food – to train soldiers from health level 0 to level 1.")
               }
            }
         },
         {
            name = "helmet_wooden",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Wodden Helmet
                  pgettext ("amazons_ware", "A wooden helmet is a basic equipment to protect soldiers. It is produced by the dressmaker and used to train soldiers from health level 1 to level 2."),
               }
            }
         },
         {
            name = "warriors_coat",
            default_target_quantity = 1,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Warriors Coat
                  pgettext("amazons_ware", "Ordinary tunics can be decorated and reinforced with gold and wood chunks. Such clothes are the best armor.")
               }
            }
         },
         {
            name = "tunic",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Tunic
                  pgettext("amazons_ware", "Rubber can be bound into a tunic. They are used as basic armor. All new soldiers are clothed in a tunic.")
               }
            }
         },
         {
            name = "vest_padded",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Padded Vest
                  pgettext("amazons_ware", "A padded vest is a light but durable armor. it is produced by the dressmaker and used to train soldiers defence from level 0 to level 1.")
               }
            }
         },
         {
            name = "protector_padded",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Padded Protector
                  pgettext("amazons_ware", "A padded protector is a strong and durable armor. It is produced by the dressmaker and used to train soldiers defence from level 1 to level 2.")
               }
            }
         },
         {
            name = "boots_sturdy",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Sturdy Boots
                  pgettext("amazons_ware", "Sturdy boots are an excellent footwear for soldiers to be quicker on the feet. They are produced by the dressmaker and used to train soldiers evade from level 0 to level 1.")
               }
            }
         },
         {
            name = "boots_swift",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Swift Boots
                  pgettext("amazons_ware", "Swift boots are a light and durable footwear for soldiers to be quicker on the feet. They are produced by the dressmaker and used to train soldiers evade from level 1 to level 2.")
               }
            }
         },
                  {
            name = "boots_hero",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an amazon ware: Hero Boots
                  pgettext("amazons_ware", "Hero boots! The only suitable footwear for a real hero. They are produced by the dressmaker and used to train soldiers evade from level 2 to level 3.")
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
               -- TRANSLATORS: Helptext for an amazon worker: Carrier
               purpose = pgettext("amazons_worker", "Carries items along your roads.")
            }
         },
         {
            name = "amazons_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Ferry
               purpose = pgettext("amazons_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "amazons_tapir",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Tapir
               purpose = pgettext("amazons_worker", "Tapirs help to carry items along busy roads. They are reared in a tapir farm.")
            }
         },
         {
            name = "amazons_tapir_breeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Tapir Breeder
               purpose = pgettext("amazons_worker", "Breeds the tapirs for adding them to the transportation system.")
            }
         }
      },
      {
         -- Building Materials
         {
            name = "amazons_stonecutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_woodcutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Woodcutter
               purpose = pgettext("amazons_worker", "Fells trees.")
            }
         },
         {
            name = "amazons_woodcutter_master",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Master Woodcutter
               purpose = pgettext("amazons_worker", "Harvests special trees: ironwood, rubber and balsa.")
            }
         },
         {
            name = "amazons_jungle_preserver",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Jungle Preserver
               purpose = pgettext("amazons_worker", "Plants trees.")
            }
         },
         {
            name = "amazons_jungle_master",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Jungle Master
               purpose = pgettext("amazons_worker", "Plants special trees: ironwood, rubber and balsa.")
            }
         },
         {
            name = "amazons_liana_cutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Liana Cutter
               purpose = pgettext("amazons_worker", "Cuts lianas from trees.")
            }
         },
         {
            name = "amazons_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Builder
               purpose = pgettext("amazons_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "amazons_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Shipwright
               purpose = pgettext("amazons_worker", "Works at the shipyard and constructs new ships.")
            }
         },
      },
      {
         -- Food
         {
            name = "amazons_hunter_gatherer",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Hunter Gatherer
               purpose = pgettext("amazons_worker", "The hunter gatherer brings fresh fish and raw meat for the tribe.")
            }
         },
         {
            name = "amazons_cassava_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Cassava Farmer
               purpose = pgettext("amazons_worker", "Grews cassava roots.")
            }
         },
         {
            name = "amazons_cocoa_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Cocoa Farmer
               purpose = pgettext("amazons_worker", "Grews cocoa.")
            }
         },
         {
            name = "amazons_cook",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Cook
               purpose = pgettext("amazons_worker", "Prepares rations, bakes cassava bread and brews chocolate.")
            }
         },
         {
            name = "amazons_wilderness_keeper",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Wilderness Keeper
               purpose = pgettext("amazons_worker", "Breeds game or fish.")
            }
         },
      },
      {
         -- Mining
         {
            name = "amazons_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Geologist
               purpose = pgettext("amazons_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "amazons_gold_digger",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Gold Digger
               purpose = pgettext("amazons_worker", "Works hard in the gold dwellings in mountains to obtain gold by washing it from gravel.")
            }
         },
         {
            name = "amazons_charcoal_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Charcoal Burner
               purpose = pgettext("amazons_worker", "Burns logs to produce charcoal.")
            }
         },
         {
            name = "amazons_gold_smelter",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Gold Smelter
               purpose = pgettext("amazons_worker", "Smelts gold at furnaces.")
            }
         },
      },
      {
         -- Tools
         {
            name = "amazons_stonecarver",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Stonecarver
               purpose = pgettext("amazons_worker", "Produces spears for soldiers and tools for workers.")
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
               -- TRANSLATORS: Helptext for an amazon worker: Soldier
               purpose = pgettext("amazons_worker", "Defend and Conquer!")
            }
         },
         {
            name = "amazons_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Trainer
               purpose = pgettext("amazons_worker", "Trains the soldiers.")
            }
         },
         {
            name = "amazons_dressmaker",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Dressmaker
               purpose = pgettext("amazons_worker", "Produces all kinds of dresses, boots and armor.")
            }
         },
         {
            name = "amazons_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for an amazon worker: Scout
               purpose = pgettext("amazons_worker", "Explores unknown territory.")
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
            -- TRANSLATORS: Helptext for an atlantean immovable: Cassavaroot Field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "cassavarootfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Cassavaroot Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cassavarootfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Cassavaroot Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cassavarootfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Casavaroot Field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "cassavarootfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Cassavaroot Field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "cocoa_tree_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable: Cocoa Tree
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
            -- TRANSLATORS: Helptext for an amazon resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "amazons_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon resource indicator: Water
            purpose = _("There is water in the ground here.")
         }
      },
      {
         name = "amazons_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an amazon resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold digger dwellings."),
               -- TRANSLATORS: Helptext for an amazon resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "amazons_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an amazon resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a stone mine."),
               -- TRANSLATORS: Helptext for an amazon resource indicator: Stones, part 2
               _("There is only a little bit of granite here."),
            }
         }
      },
      {
         name = "amazons_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an amazon resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold digger dwellings."),
               -- TRANSLATORS: Helptext for an amazon resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "amazons_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an amazon resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a stone mine."),
               -- TRANSLATORS: Helptext for an amazon resource indicator: Stones, part 2
               _("There is a lot of granite here.")
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
            -- TRANSLATORS: Purpose helptext for an amazon warehouse: Headquarters
            purpose = pgettext("amazons_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for an amazon warehouse: Headquarters
            note = pgettext("amazons_building", "The headquarters is your main building.")
         }
      },
      {
         name = "amazons_warehouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an amazon warehouse: Warehouse
            purpose = pgettext("building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "amazons_port",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an amazon warehouse: Port
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
