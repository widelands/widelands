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
-- The worker will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``.
--
-- .. function:: new_worker_type{table}
--
--    This function adds the definition of a worker to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--                to this worker. It contains the properties in
--                :ref:`lua_tribes_workers_common`.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
-- push_textdomain("tribes")

-- dirname = path.dirname(__file__)

-- wl.Descriptions():new_worker_type {
   -- name = "empire_fisher",
   -- -- TRANSLATORS: This is a worker name used in lists of workers
   -- descname = pgettext("empire_worker", "Fisher"),
   -- animation_directory = dirname,
   -- icon = dirname .. "menu.png",
   -- vision_range = 2,

   -- buildcost = {
      -- empire_carrier = 1,
      -- fishing_rod = 1
   -- },

   -- programs = {
      -- fish = {
         -- "findspace=size:any radius:7 resource:resource_fish",
         -- "walk=coords",
         -- "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
         -- "mine=resource_fish radius:1",
         -- "animate=fishing duration:10s", -- Play a fishing animation
         -- "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
         -- "createware=fish",
         -- "return"
      -- }
   -- },

   -- animations = {
      -- idle = {
         -- hotspot = { 7, 38 },
      -- },
   -- },

   -- spritesheets = {
      -- fishing = {
         -- fps = 10,
         -- frames = 30,
         -- rows = 6,
         -- columns = 5,
         -- hotspot = { 9, 39 }
      -- },
      -- walk = {
         -- fps = 20,
         -- frames = 20,
         -- rows = 5,
         -- columns = 4,
         -- directional = true,
         -- hotspot = { 10, 38 }
      -- },
      -- walkload = {
         -- basename = "walk",
         -- fps = 20,
         -- frames = 20,
         -- rows = 5,
         -- columns = 4,
         -- directional = true,
         -- hotspot = { 10, 38 }
      -- },
   -- },
-- }

-- pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_armorsmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Armorsmith"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   animations = {
      idle = {
         hotspot = { 10, 21 }
      },
   },

   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 23 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 23 }
      },
   },
}

pop_textdomain()
