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
-- The building will also need its help texts, which are defined in
-- ``buildings/partially_finished/constructionsite/helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_constructionsite_type{table}
--
--    This function adds the definition of the construction site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It only contains :ref:`lua_tribes_buildings_common`.
--
tribes:new_constructionsite_type {
   msgctxt = "building",
   name = "constructionsite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings under construction
   descname = pgettext("building", "Construction Site"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = {
      -- The constructionsite is a mess. Not nice and clean, but rather some
      -- logs lying around on piles, maybe some tools.
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 5, 5 },
      },
      idle_with_worker = {
         pictures = path.list_files(dirname .. "idle_with_worker_??.png"),
         hotspot = { 33, 36 },
      }
   },

   aihints = {},
}
