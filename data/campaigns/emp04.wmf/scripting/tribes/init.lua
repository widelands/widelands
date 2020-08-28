-- RST
-- .. _scenario_units:
--
-- Scenario-Specific Units
-- -----------------------
--
-- When writing a scenario, you can create custom tribe objects for it.
-- At the moment, we support buildings, wares and workers.
-- If you need support for other types of tribe units, please open a new bug report.
--
-- For creating scenario-specific tribe objects, create a subdirectory
-- ``scripting/tribes`` in your map.
-- In that subdirectory, place a file called ``init.lua``.
-- This file needs to return a table listing all the objects that the tribes do
-- not have by default. You can also add helptexts. For example:
--
-- .. code-block:: lua
--
--    return {
--       empire = {
--          buildings = {
--             {
--                name = "empire_brewery1",
--                helptexts = {
--                   -- TRANSLATORS: Purpose helptext for an empire production site: Brewery
--                   purpose = pgettext(buildingctxt, "Produces beer to keep the miners strong and happy.")
--                }
--             },
--             name = "empire_trainingcamp1",
--                helptexts = {
--                   purpose = {
--                      -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 1
--                      pgettext(buildingctxt, "Trains soldiers in ‘Attack’ and in ‘Health’."),
--                      -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 2
--                      pgettext(buildingctxt, "Equips the soldiers with all necessary weapons and armor parts.")
--                   },
--                   -- TRANSLATORS: Note helptext for an empire training site: Training Camp
--                   note = pgettext(buildingctxt, "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
--                }
--             },
--             {
--                name = "empire_temple_of_vesta",
--                helptexts = {
--                   -- TRANSLATORS: Purpose helptext for an empire warehouse: Temple of Vesta
--                   purpose = pgettext(buildingctxt, "In the temple of Vesta, the wares to worship are stored. It is the home of the priestesses and the guards of the goddess.")
--                }
--             }
--          }
--          workers_order = {
--             {
--                {
--                   name = "empire_vesta_priestess",
--                   default_target_quantity = 1,
--                   preciousness = 15
--                }
--             },
--          },
--          wares_order = {
--             {
--                {
--                   name = "empire_vesta_wine"
--                   default_target_quantity = 10,
--                   preciousness = 4
--                }
--             },
--          }
--       },
--       frisians = {
--          buildings = {
--             "frisians_warehouse1"
--          }
--       }
--    }
--
-- If you are only replacing already existing objects, you can skip adding the
-- ``init.lua`` file.
--
-- Now place your custom tribe objects into subdirectories below ``scripting/tribes``:
--
-- * In each subdirectory, there needs to be an ``init.lua`` file that adds all
--   objects for that subdirectory.
--   If an object has the same name as an already existing tribe object,
--   it will replace that tribe object.
-- * You will also need to add a ``register.lua`` file that lists the names of all
--   objects that you are adding or replacing.
-- * You can also add ``.png`` files to your subdirectories.
-- * Adding helptexts is optional, but will units without a helptext will print a warning on the console.
--
-- See :ref:`defining_tribe_units` for further details on these files, and :ref:`lua_tribes_tribes_helptexts` on the helptext format.

-- For formatting time strings
include "tribes/scripting/help/global_helptexts.lua"

local buildingctxt = "empire_building"

push_textdomain("scenario_emp04.wmf")

local result = {
   empire = {
      buildings = {
         {
            name = "empire_brewery1",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an empire production site: Brewery
               purpose = pgettext(buildingctxt, "Produces beer to keep the miners strong and happy.")
            }
         },
         {
            name = "empire_brewery2",
         },
         {
            name = "empire_farm1",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an empire production site: Farm
               purpose = pgettext(buildingctxt, "Sows and harvests wheat.")
            }
         },
         {
            name = "empire_farm2",
         },
         {
            name = "empire_foresters_house1",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an empire production site: Forester's House
               purpose = pgettext(buildingctxt, "Plants trees in the surrounding area."),
               -- TRANSLATORS: Note helptext for an empire production site: Forester's House
               note = pgettext(buildingctxt, "The forester’s house needs free space within the work area to plant the trees.")
            }
         },
         {
            name = "empire_lumberjacks_house1",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an empire production site: Lumberjack's House
               purpose = pgettext(buildingctxt, "Fells trees in the surrounding area and processes them into logs."),
               -- TRANSLATORS: Note helptext for an empire production site: Lumberjack's House
               note = pgettext(buildingctxt, "The lumberjack's house needs trees to fell within the work area.")
            }
         },
         {
            name = "empire_mill1",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an empire production site: Mill
               purpose = pgettext(buildingctxt, "Grinds wheat to produce flour."),
            }
         },
         {
            name = "empire_mill2",
         },
         {
            name = "empire_trainingcamp1",
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 1
                  pgettext(buildingctxt, "Trains soldiers in ‘Attack’ and in ‘Health’."),
                  -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 2
                  pgettext(buildingctxt, "Equips the soldiers with all necessary weapons and armor parts.")
               },
               -- TRANSLATORS: Note helptext for an empire training site: Training Camp
               note = pgettext(buildingctxt, "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
            }
         },
         {
            name = "empire_well1",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an empire production site: Well
               purpose = pgettext(buildingctxt, "Draws water out of the deep.")
            }
         },
         {
            name = "empire_temple_of_vesta",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an empire warehouse: Temple of Vesta
               purpose = pgettext(buildingctxt, "In the temple of Vesta, the wares to worship are stored. It is the home of the priestesses and the guards of the goddess.")
            }
         }
      }
   }
}

pop_textdomain()
return result
