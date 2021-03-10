-- RST
-- .. _lua_tribes_carriers:
--
-- Carriers
-- --------
--
-- Carriers are specialized workers that carry wares along the roads.
-- Each tribe has a basic human carrier + a second carrier for busy roads -
-- usually a beast of burden.
--
-- Carriers, like workers, are defined in
-- ``data/tribes/workers/<tribe name>/<worker_name>/init.lua``.
-- The carrier will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
-- .. function:: new_carrier_type{table}
--
--    This function adds the definition of a carrier to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--                to this carrier. It contains the :ref:`lua_tribes_workers_common`.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("tribes")
--
--    descriptions:new_carrier_type {
--       name = "atlanteans_carrier",
--       descname = pgettext("atlanteans_worker", "Carrier"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       vision_range = 2,
--
--       buildcost = {}, -- This will give the worker the property "buildable"
--
--       animations = {
--          idle = {
--             hotspot = { 13, 24 },
--             fps = 10
--          },
--          walk = {
--             hotspot = { 8, 25 },
--             fps = 10,
--             directional = true
--          },
--          walkload = {
--             hotspot = { 8, 25 },
--             fps = 10,
--             directional = true
--          }
--       }
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_carrier_type {
   name = "atlanteans_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Carrier"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = {
      idle = {
         hotspot = { 13, 24 },
         fps = 10
      },
      walk = {
         hotspot = { 8, 25 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 25 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
