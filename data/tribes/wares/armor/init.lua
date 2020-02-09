-- RST
-- .. _lua_tribes_wares:
--
-- Wares
-- =====
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
--    ``{ atlanteans = 0, empire = 1 }``. We recommend not going higher than ``25``.
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

-- RST
--
-- Help Texts
-- ----------
--
-- Each ware has a ``helptexts.lua`` script, which is located in the same directory as its ``init.lua`` script.
-- The function in this file returns texts that are used for the ware by the Tribal Encyclopedia.
--
-- .. function:: ware_helptext([tribe])
--
--    Returns a localized string with a helptext for this ware type.
--
--    :arg tribe: the name of the tribe to fetch this helptext for.
--    :type tribe: :class:`string`
--
--    If you call this function without a tribe name, it will deliver a default
--    help text, if present. Both a default and a tribe helptext can be defined
--    at the same time, and they are designed in such a way that they are supposed
--    to be concatenated. Example call of this function::
--
--        pgettext("sentence_separator", "%s %s"):bformat(ware_helptext(), ware_helptext(tribename))
--
--    If you wish to define helptexts of your own, best copy over this function
--    from another ware and then fill in the data.
--
