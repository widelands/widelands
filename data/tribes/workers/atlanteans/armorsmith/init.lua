-- The basic worker documentation is located in /doc/sphinx/lua_tribes_workers_rst.org

-- RST
-- .. _lua_tribes_basic_workers:
--
-- Workers
-- --------
--
-- Workers of this basic type work in buildings. Their function is defined either
-- through the :ref:`buildings' programs <productionsite_programs>`, or through their :ref:`own programs <tribes_worker_programs>`.
--
-- Workers are defined in
-- ``data/tribes/workers/<tribe name>/<worker_name>/init.lua``.
-- The worker will also need its :ref:`help texts <lua_tribes_workers_helptexts>`,
-- which are defined in ``data/tribes/wares/<tribe name>/<worker_name>/helptexts.lua``.
dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 21 },
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {8, 23}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {8, 23}, 10)

-- RST
-- .. function:: new_worker_type{table}
--
--    This function adds the definition of a worker to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--                to this worker. It contains the properties in
--                :ref:`lua_tribes_workers_common`.
tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_armorsmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Armorsmith"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   animations = animations,
}
