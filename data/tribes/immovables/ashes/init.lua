-- RST
-- .. _lua_tribes_immovables:
--
-- Immovables
-- ==========
--
-- Immovables are entities not connected to a road that a tribe can own,
-- for example a corn field or a ship under construction.
--
-- Tribe Immovables are defined in
-- ``data/tribes/immovables/<immovable_name>/init.lua``.

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_immovable_type(table)
--
--    This function adds the definition of a tribe immovable to the engine.
--
--    :arg table: This table contains all the data that the game engine will
--                add to this immovable. It contains the following entries:
--
--    **msgctxt**: The context that Gettext will use to disambiguate the
--    translations for strings in this table.
--
--    **name**: A string containing the internal name of this immovable.
--
--    **descname**: The translatable display name. Use ``pgettext`` with the
--    ``msgctxt`` above to fetch the string.
--
--    **helptext_script**:  The full path to the ``helptexts.lua`` script for this immovable.
--
--    **programs**: A table with the programs that this immovable will perform,
--    e.g. ``{ "animate=idle duration:2m30s500ms",  "remove=" }`` (see :ref:`immovable_programs`)
--
--    **animations**: A table containing all animations for this immovable.
--
tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "ashes",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ashes"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:45s",
         "remove=",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 40, 39 },
      },
   }
}


-- RST
--
-- Help Texts
-- ----------
--
-- Each tribe immovable has a ``helptexts.lua`` script, which is located in the same directory as its ``init.lua`` script.
-- The function in this file returns texts that are used for the immovable by the Tribal Encyclopedia.
--
-- .. function:: immovable_helptext([tribe])
--
--    Returns a localized string with a helptext for this immovable type.
--
--    :arg tribe: the name of the tribe to fetch this helptext for.
--    :type tribe: :class:`string`
--
--    This function works exactly the same as :any:`ware_helptext`.
--
