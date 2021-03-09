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
--
-- .. function:: new_immovable_type(table)
--
--    This function adds the definition of a tribe immovable to the engine.
--
--    :arg table: This table contains all the data that the game engine will
--                add to this immovable. It contains the following entries:
--
--    **name**: A string containing the internal name of this immovable.
--
--    **descname**: The translatable display name. Use ``pgettext`` to fetch the string.
--
--    **helptext_script**:  The full path to the ``helptexts.lua`` script for this immovable.
--    **NOTE: Deprecated. Immovable helptexts have been shifted to tribes initialization in the current
--    development version.**
--
--    **programs**: A table with the programs that this immovable will perform,
--    e.g. ``{ "animate=idle duration:2m30s500ms",  "remove=" }`` (see :ref:`immovable_programs`)
--
--    **animations**: A table containing all animations for this immovable.
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
--    descriptions:new_immovable_type {
--       name = "ashes",
--       descname = pgettext("immovable", "Ashes"),
--       anmation_directory = dirname,
--       icon = dirname .. "menu.png",
--       size = "small",
--       programs = {
--          main = {
--             "animate=idle duration:45s",
--             "remove=",
--          }
--       },
--
--       animations = {
--          idle = {
--             hotspot = { 40, 39 },
--          },
--       }
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "ashes",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ashes"),
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

pop_textdomain()
