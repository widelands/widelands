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
-- not have by default. For example:
--
-- .. code-block:: lua
--
--    return {
--       empire = {
--          buildings = {
--             "empire_brewery1",
--             "empire_brewery2",
--             "empire_well1",
--             "empire_temple_of_vesta"
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
--       helptexts = {
--          buildings = {
--             empire_temple_of_vesta = {
--                purpose = {
--                   -- TRANSLATORS: Purpose helptext for an empire warehouse: Temple of Vesta
--                   pgettext(buildingctxt, "In the temple of Vesta, the wares to worship are stored. It is the home of the priestesses and the guards of the goddess.")
--                }
--             },
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
include "tribes/scripting/help/time_strings.lua"

local buildingctxt = "empire_building"

push_textdomain("scenario_emp04.wmf")

local brewery_helptexts = {
   purpose = {
      -- TRANSLATORS: Purpose helptext for an empire production site: Brewery
      pgettext(buildingctxt, "Produces beer to keep the miners strong and happy.")
   },
   performance = {
      -- TRANSLATORS: Performance helptext for an empire production site: Brewery
      pgettext(buildingctxt, "The brewer needs %1% on average to brew a vat of beer."):bformat(format_minutes_seconds(1, 5))
   }
}
local farm_helptexts = {
   purpose = {
      -- TRANSLATORS: Purpose helptext for an empire production site: Farm
      pgettext(buildingctxt, "Sows and harvests wheat.")
   },
   performance = {
      -- TRANSLATORS: Performance helptext for an empire production site: Farm
      pgettext(buildingctxt, "The farmer needs %1% on average to sow and harvest a sheaf of wheat."):bformat(format_minutes_seconds(1, 20))
   }
}
local mill_helptexts = {
   purpose = {
      -- TRANSLATORS: Purpose helptext for an empire production site: Mill
      pgettext(buildingctxt, "Grinds wheat to produce flour.")
   },
   performance = {
      -- TRANSLATORS: Performance helptext for an empire production site: Mill
      pgettext(buildingctxt, "The miller needs %1% on average to grind wheat into a sack of flour."):bformat(format_seconds(19))
   }
}

local result = {
   empire = {
      buildings = {
         "empire_brewery1",
         "empire_brewery2",
         "empire_farm1",
         "empire_farm2",
         "empire_foresters_house1",
         "empire_lumberjacks_house1",
         "empire_mill1",
         "empire_mill2",
         "empire_trainingcamp1",
         "empire_well1",
         "empire_temple_of_vesta"
      },
      helptexts = {
         buildings = {
            empire_temple_of_vesta = {
               purpose = {
                  -- TRANSLATORS: Purpose helptext for an empire warehouse: Temple of Vesta
                  pgettext(buildingctxt, "In the temple of Vesta, the wares to worship are stored. It is the home of the priestesses and the guards of the goddess.")
               }
            },
            empire_brewery1 = brewery_helptexts,
            empire_brewery2 = brewery_helptexts,
            empire_farm1 = farm_helptexts,
            empire_farm2 = farm_helptexts,
            empire_foresters_house1 = {
               purpose = {
                  -- TRANSLATORS: Purpose helptext for an empire production site: Forester's House
                  pgettext(buildingctxt, "Plants trees in the surrounding area.")
               },
               note = {
                  -- TRANSLATORS: Note helptext for an empire production site: Forester's House
                  pgettext(buildingctxt, "The forester’s house needs free space within the work area to plant the trees.")
               }
            },
            empire_lumberjacks_house1 = {
               purpose = {
                  -- TRANSLATORS: Purpose helptext for an empire production site: Lumberjack's House
                  pgettext(buildingctxt, "Fells trees in the surrounding area and processes them into logs.")
               },
               note = {
                  -- TRANSLATORS: Note helptext for an empire production site: Lumberjack's House
                  pgettext(buildingctxt, "The lumberjack's house needs trees to fell within the work area.")
               }
            },
            empire_mill1 = mill_helptexts,
            empire_mill2 = mill_helptexts,
            empire_trainingcamp1 = {
               purpose = {
                  -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 1
                  pgettext(buildingctxt, "Trains soldiers in ‘Attack’ and in ‘Health’."),
                  -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 2
                  pgettext(buildingctxt, "Equips the soldiers with all necessary weapons and armor parts.")
               },
               note = {
                  -- TRANSLATORS: Note helptext for an empire training site: Training Camp
                  pgettext(buildingctxt, "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
               }
            },
            empire_well1 = {
               purpose = {
                  -- TRANSLATORS: Purpose helptext for an empire production site: Well
                  pgettext(buildingctxt, "Draws water out of the deep.")
               }
            },
         }
      }
   }
}

pop_textdomain()
return result
