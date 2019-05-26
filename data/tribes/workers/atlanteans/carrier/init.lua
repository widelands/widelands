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

animations = {}
add_animation(animations, "idle", dirname, "idle", {13, 24}, 10)
add_directional_animation(animations, "walk", dirname, "walk", {8, 25}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {8, 25}, 10)

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
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}
