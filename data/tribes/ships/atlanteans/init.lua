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
--
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
--    **animations**: A table containing all animations for this ship.
--    Ships have an "idle", a "sinking" and a directional "sail" animation.
--
--    **names**: A list of strings with ship names presented to the user - be creative :)

push_textdomain("tribes")

dirname = path.dirname(__file__)

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
   },

   names = {
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Abaco"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Agate"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Alexandrite"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Amber"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Amethyst"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Anguilla"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Antigua"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Aquamarine"),
      -- TRANSLATORS: This is an Atlantean ship name
      pgettext("shipname", "Atlantean’s Stronghold"),
      -- TRANSLATORS: This is an Atlantean ship name
      pgettext("shipname", "Atlantis"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Bahama"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Barbados"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Barbuda"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Beryl"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Blanquilla"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Caicos"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Cassiterite"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Cat’s Eye"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Citrine"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Colionder"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Dominica"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Eleuthera"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Emerald"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Grenada"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Guadeloupe"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Inagua"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Jundlina"),
      -- TRANSLATORS: This is an Atlantean ship name
      pgettext("shipname", "Juventud"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "King Ajanthul"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "King Askandor"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Kitts"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Loftomor"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Malachite"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Martinique"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Montserrat"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Moonstone"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Mystic Quartz"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Nassau"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Nevis"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Obsidian"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Onyx"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Opol"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Orchila"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Ostur"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Pearl"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Sapphire"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Satul"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Sidolus"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Sphalerite"),
      -- TRANSLATORS: This is an Atlantean ship name
      pgettext("shipname", "Spider"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Spinel"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Sunstone"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Tiger Eye"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Tobago"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Topaz"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Tortuga"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Tourmaline"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Trinidad"),
   }
}

pop_textdomain()
