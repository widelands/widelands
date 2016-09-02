-- RST
-- buildings/partially_finished/constructionsite/init.lua
-- ------------------------------------------------------
--
-- The constructionsite is a special building.
-- It is a building site where a building is being constructed.
-- It is defined only once for all tribes.
-- The building will also need its helptexts, which are defined in ``helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_constructionsite_type(table)
--
--    This function adds the definition of the construction site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It contains the following entries:
--
--    **msgctxt**: The context that Gettext will use to disambiguate the translations for strings in this table.
--
--    **name**: A string containing the internal name of this building.
--
--    **descname**: The translatable display name. use ``pgettext`` with the ``msgctxt`` above to fetch the string.
--
--    **helptext_script**: The full path to the ``helptexts.lua`` script for this building.
--
--    **icon**: The full path to the menu icon for this building.
--
--    **vision_range**: The size of the radious that the building sees.
--
--    **size**: The size of this building: ``"small"``, ``"medium"``, or ``"big"``.
--
--    **animations**: A table containing all animations for this building.
--
--    **aihints**: A list of name - value pairs with hints for the AI.
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
