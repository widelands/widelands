-- RST
-- .. _lua_tribes_wares:
--
-- Wares
-- -----
--
-- Wares are used to build buildings, produce other wares and recruit units.
--
-- Wares are defined in
-- ``data/tribes/wares/<ware_name>/init.lua``.
-- The ware will also need its help texts, which are defined in
-- ``data/tribes/wares/<ware_name>/helptexts.lua``
--
-- Fetching the helptext for a ware depends on the current tribe. So, best copy
-- the function out of ``data/tribes/wares/bread_paddle/helptexts.lua``
-- and use it as a base for creating your ware's helptexts.

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_ware_type(table)
--
--    This function adds the definition of a ware to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--                to this ware. It contains the following entries:
--
--    **msgctxt**: The context that Gettext will use to disambiguate the
--    translations for strings in this table.
--
--    **name**: A string containing the internal name of this ware.
--
--    **descname**: The translatable display name. Use ``pgettext`` with the
--    ``msgctxt`` above to fetch the string.
--
--    **helptext_script**: The full path to the ``helptexts.lua`` script for this ware.
--
--    **icon**: The full path to the menu icon for this ware.
--
--    **default_target_quantity**: A table listing the default target quantity
--    for each tribe's economy. For example, ``{ atlanteans = 3, empire = 1 }``
--
--    **preciousness**: How precious this ware is to each tribe. For example,
--    ``{ atlanteans = 0, empire = 1 }``
--
--    **animations**: A table containing all animations for this ware.
--    Wares have an "idle" animation.
--
tribes:new_ware_type {
   msgctxt = "ware",
   name = "armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Armor"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 1
   },
   preciousness = {
      empire = 0
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 11 },
      },
   }
}
