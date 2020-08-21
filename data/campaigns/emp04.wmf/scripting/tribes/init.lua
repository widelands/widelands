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
--                "empire_vesta_priestess"
--             },
--          },
--          wares_order = {
--             {
--                "empire_vesta_wine"
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
-- * You can also add ``helptexts.lua`` and ``.png`` files to your subdirectories.
--
-- See :ref:`defining_tribe_units` for further details on these files.

return {
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
      }
   }
}
