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
--    push_textdomain("tribes")
--
--    tribes:new_worker_type {
--       name = "empire_fisher",
--       descname = pgettext("empire_worker", "Fisher"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       vision_range = 2,
--
--       buildcost = {
--          empire_carrier = 1,
--          fishing_rod = 1
--       },
--
--       programs = {
--          fish = {
--             "findspace=size:any radius:7 resource:fish",
--             "walk=coords",
--             "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
--             "mine=resource_fish radius:1",
--             "animate=fishing duration:10s", -- Play a fishing animation
--             "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
--             "createware=fish",
--             "return"
--          }
--       },
--
--       animations = {
--          idle = {
--             hotspot = { 7, 38 },
--          },
--          fishing = {
--             hotspot = { 9, 39 },
--             fps = 10,
--          },
--          walk = {
--             hotspot = { 10, 38 },
--             fps = 20,
--             directional = true
--          },
--          walkload = {
--             basename = "walk",
--             hotspot = { 10, 38 },
--             fps = 20,
--             directional = true
--          }
--       }
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "atlanteans_armorsmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Armorsmith"),
   helptext_script = dirname .. "helptexts.lua",
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
      walk = {
         hotspot = { 8, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
