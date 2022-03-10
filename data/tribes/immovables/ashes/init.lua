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
-- The immovable will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
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
--    **programs**: A table with the programs that this immovable will perform,
--    e.g. ``{ "animate=idle duration:2m30s500ms",  "remove=" }`` (see :ref:`immovable_programs`)
--
--    **animations**
--        *Optional*. A table containing all file animations for this immovable.
--        Every immovable needs to have at least an ``idle`` animation.
--        Animations can either be defined as file animations in this table or as spritesheet animations
--        as defined in table ``spritesheets``. A mixture of the two animation formats is allowed.
--        See :doc:`animations` for a detailed description of the animation format.
--
--    **spritesheets**
--        *Optional*. A table containing all spritesheet animations for this immovable.
--        Every immovable needs to have at least an ``idle`` animation.
--        Animations can either be defined as spritesheet animations in this table or as file animations
--        as defined in table ``animations``. A mixture of the two animation formats is allowed.
--        See :doc:`animations` for a detailed description of the animation format.
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
--    wl.Descriptions():new_immovable_type {
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

wl.Descriptions():new_immovable_type {
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 40, 39 },
      },
   }
}

pop_textdomain()

-- RST
--
-- Help Texts
-- ----------
--
-- See :ref:`lua_tribes_tribes_helptexts`.
--
