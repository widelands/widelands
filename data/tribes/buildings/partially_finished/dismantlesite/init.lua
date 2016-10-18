-- RST
-- Dismantle Sites
-- ---------------
--
-- The dismantlesite is a special building.
-- It is a building site where a building is being dismantled.
-- Just like the constructionsite, it is defined only once for all tribes.
--
-- The dismantlesite is defined in
-- ``buildings/partially_finished/dismantlesite/init.lua``.
-- The building will also need its help texts, which are defined in
-- ``buildings/partially_finished/dismantlesite/helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_dismantlesite_type{table}
--
--    This function adds the definition of the dismantle site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It only contains :ref:`lua_tribes_buildings_common`.
--
tribes:new_dismantlesite_type {
   msgctxt = "building",
   name = "dismantlesite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings being taken apart
   descname = pgettext("building", "Dismantle Site"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 5, 5 },
      },
   },

   aihints = {},
}
