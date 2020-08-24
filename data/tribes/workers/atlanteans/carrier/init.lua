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
-- The carrier will also need its :ref:`help texts <lua_tribes_workers_helptexts>`,
-- which are defined in ``data/tribes/wares/<tribe name>/<worker_name>/helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_carrier_type{table}
--
--    This function adds the definition of a carrier to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--                to this carrier. It contains the :ref:`lua_tribes_workers_common`.
--
tribes:new_carrier_type {
   msgctxt = "atlanteans_worker",
   --msgctxt = msgctxt,
   name = "atlanteans_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Carrier"),
   animation_directory = dirname,
   helptext_script = dirname .. "helptexts.lua",
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
