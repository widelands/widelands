-- RST
-- .. _lua_tribes_buildings_dismantlesites:
--
-- Dismantle Sites
-- ---------------
--
-- The dismantlesite is a special building.
-- It is a building site where a building is being dismantled.
-- Just like the constructionsite, it is defined only once for all tribes.
--
-- The dismantlesite is defined in
-- ``buildings/partially_finished/dismantlesite/init.lua``.
-- The building will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
-- .. function:: new_dismantlesite_type{table}
--
--    This function adds the definition of the dismantle site building to the engine.
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
--    wl.Descriptions():new_dismantlesite_type {
--       name = "dismantlesite",
--       descname = pgettext("building", "Dismantle Site"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       vision_range = 2,
--
--       animations = {
--          idle = {
--             hotspot = { 5, 5 },
--          },
--       },
--
--       aihints = {},
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_dismantlesite_type {
   name = "dismantlesite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings being taken apart
   descname = pgettext("building", "Dismantle Site"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 5, 5 },
      },
   },

   aihints = {},
}

pop_textdomain()
