-- RST
-- Tribe Immovables
-- ----------------
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
--    **programs**: A table with the programs that this immovable will perform,
--    e.g. ``{ animate=idle 4500,  remove= }``
--
--    **animations**: A table containing all animations for this building.
--
tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "ashes",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ashes"),
   programs = {
      program = {
         "animate=idle 45000",
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
