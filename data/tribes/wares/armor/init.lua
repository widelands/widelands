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
-- The ware will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
-- .. function:: new_ware_type(table)
--
--    This function adds the definition of a ware to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--                to this ware. It contains the following entries:
--
--    **name**: A string containing the internal name of this ware.
--
--    **descname**: The translatable display name. Use ``pgettext`` to fetch the string.
--
--    **icon**: The full path to the menu icon for this ware.
--
--    **default_target_quantity**: A table listing the default target quantity
--    for each tribe's economy. For example, ``{ atlanteans = 3, empire = 1 }``
--    This table does not need to have an entry for each tribe. (Can be completely empty)
--    If not set for a tribe the economy will always demand this ware.
--    If set to zero the economy will not demand this ware unless it is required in a
--    production building. If not set or set to zero the actual target quantity will
--    not be available in the economy settings window.
--    **NOTE: This parameter has been shifted to tribes initialization in the current
--    development version.**
--
--    **preciousness**: How precious this ware is to each tribe. For example,
--    ``{ atlanteans = 0, empire = 1 }``. We recommend not going higher than ``25``.
--    **NOTE: This parameter has been shifted to tribes initialization in the current
--    development version.**
--
--    **animations**: A table containing all animations for this ware.
--    Wares have an "idle" animation.
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
--    wl.Descriptions():new_ware_type {
--       name = "armor",
--       descname = pgettext("ware", "Armor"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--
--       animations = {
--          idle = {
--             hotspot = { 3, 11 },
--          },
--       }
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Armor"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 3, 11 },
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
