-- RST
-- .. _lua_tribes_ships:
--
-- Ships
-- -----
--
-- Each tribe can have one ship that is used to explore the seas and transport
-- wares between ports.
--
-- Ships are defined in
-- ``data/tribes/ships/<tribe_name>/init.lua``.

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_ship_type(table)
--
--    This function adds the definition of a ship to the engine.
--
--    :arg table: This table contains all the data that the game engine will
--                add to this ship. It contains the following entries:
--
--    **msgctxt**: The context that Gettext will use to disambiguate the
--    translations for strings in this table.
--
--    **name**: A string containing the internal name of this ship.
--
--    **descname**: The translatable display name. Use ``pgettext`` with the
--    ``msgctxt`` above to fetch the string.
--
--    **icon**: Path to the menu icon file.
--
--    **capacity**: An int defining how many wares or workers this ship can transport
--
--    **vision_range**: How far the ship can see.
--
--    **icon**: The filepath for the menu icon.
--
--    **animations**: A table containing all animations for this ship.
--    Ships have an "idle", a "sinking" and a directional "sail" animation.
tribes:new_ship_type {
   msgctxt = "atlanteans_ship",
   name = "atlanteans_ship",
   -- TRANSLATORS: This is the Atlanteans' ship's name used in lists of units
   descname = pgettext("atlanteans_ship", "Ship"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,

   animations = {
      idle = {
         hotspot = { 89, 86 },
         fps = 10
      },
      sinking = {
         hotspot = { 89, 86 },
         fps = 7
      },
      sail = {
         hotspot = { 89, 86 },
         fps = 10,
         directional = true
      }
   }
}
