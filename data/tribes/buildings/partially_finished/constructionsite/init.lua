-- RST
-- .. _lua_tribes_buildings_constructionsites:
--
-- Construction Sites
-- ------------------
--
-- The constructionsite is a special building.
-- It is a building site where a building is being constructed.
-- Just like the dismantlesite, it is defined only once for all tribes.
--
-- The constructionsite is defined in
-- ``buildings/partially_finished/constructionsite/init.lua``.
-- The building will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
-- .. function:: new_constructionsite_type{table}
--
--    This function adds the definition of the construction site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It only contains :ref:`lua_tribes_buildings_common`.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("tribes")
--
--    dirname = path.dirname(__file__)
--
--    wl.Descriptions():new_constructionsite_type {
--       name = "constructionsite",
--       descname = pgettext("building", "Construction Site"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       vision_range = 2,
--
--       animations = {
--          idle = {
--             hotspot = { 5, 5 },
--          },
--          idle_with_worker = {
--             hotspot = { 33, 36 },
--          }
--       },
--
--       aihints = {},
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_constructionsite_type {
   name = "constructionsite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings under construction
   descname = pgettext("building", "Construction Site"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   animations = {
      -- The constructionsite is a mess. Not nice and clean, but rather some
      -- logs lying around on piles, maybe some tools.
      idle = {
         hotspot = { 5, 5 },
      },
      idle_with_worker = {
         hotspot = { 33, 36 },
      }
   },

   aihints = {},
}

pop_textdomain()
