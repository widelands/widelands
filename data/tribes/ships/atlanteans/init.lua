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
--    **name**: A string containing the internal name of this ship.
--
--    **descname**: The translatable display name. Use ``pgettext`` to fetch the string.
--
--    **icon**: Path to the menu icon file.
--
--    **capacity**: An int defining how many wares or workers this ship can transport
--
--    **vision_range**: How far the ship can see.
--
--    **animations**: A table containing all file animations for this ship.
--    Ships have an "idle", a "sinking" and a directional "sail" animation.
--    Animations can either be defined as file animations in this table or as spritesheet animations
--    as defined in table ``spritesheets``. A mixture of the two animation formats is allowed.
--    See :doc:`animations` for a detailed description of the animation format.
--
--    **spritesheets**: A table containing all spritesheet animations for this ship.
--    Ships have an "idle", a "sinking" and a directional "sail" animation.
--    Animations can either be defined as spritesheet animations in this table or as file animations
--    as defined in table ``animations``. A mixture of the two animation formats is allowed.
--    See :doc:`animations` for a detailed description of the animation format.
--
--    **names**: A list of strings with ship names presented to the user - be creative :)
--
--    **port_names**: A list of strings with port names presented to the user - be creative :)
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
--    wl.Descriptions():new_ship_type {
--       name = "atlanteans_ship",
--       descname = pgettext("atlanteans_ship", "Ship"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       capacity = 30,
--       vision_range = 4,
--
--       spritesheets = {
--          idle = {
--             fps = 10,
--             frames = 39,
--             rows = 7,
--             columns = 6,
--             hotspot = { 58, 55 }
--          },
--          sail = {
--             fps = 10,
--             frames = 40,
--             rows = 7,
--             columns = 6,
--             directional = true,
--             hotspot = { 86, 85 }
--          },
--          sinking = {
--             fps = 7,
--             frames = 22,
--             rows = 6,
--             columns = 4,
--             hotspot = { 58, 54 }
--          },
--       },
--       names = {
--          pgettext("shipname", "Abaco"),
--          pgettext("shipname", "Agate"),
--       },
--       port_names = {
--          pgettext("portname", "Kos"),
--          pgettext("portname", "Patmos"),
--       }
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ship_type {
   name = "atlanteans_ship",
   -- TRANSLATORS: This is the Atlanteans' ship's name used in lists of units
   descname = pgettext("atlanteans_ship", "Ship"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,

   spritesheets = {
      idle = {
         fps = 10,
         frames = 39,
         rows = 7,
         columns = 6,
         hotspot = { 58, 55 }
      },
      sail = {
         fps = 10,
         frames = 40,
         rows = 7,
         columns = 6,
         directional = true,
         hotspot = { 86, 85 }
      },
      sinking = {
         fps = 7,
         frames = 22,
         rows = 6,
         columns = 4,
         hotspot = { 58, 54 }
      },
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
   },
   port_names = {
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Aegina"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Constantinos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kirikos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Aggistri"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Alonissos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Anafi"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Andros"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Antikythira"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Arkyi"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Astypalea"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Chalki"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Chania"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Chios"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Diafani"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Donoussa"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Euboea"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Folegandros"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Fourni"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Gytheio"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Heraklion"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Hydra"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Igoumenitsa"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Ios"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Iraklia"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Ithaki"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kalamata"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kalymnos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Karlovassi"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Karpathos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kasos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kastelorizo"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Katapola"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kavala"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kefalonia"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kilini"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kimolos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kissamos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Korfu"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Koufonissi"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kythira"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Kythnos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Lavrio"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Leros"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Lesvos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Limnos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Lipsi"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Milos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Mykonos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Naxos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Nisyros"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Paros"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Patmos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Patras"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Paxoi"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Piraeus"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Poros"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Rafina"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Rhodos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Samos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Santorini"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Schinoussa"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Serifos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Sifnos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Sikinos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Sitia"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Skiathos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Skopelos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Symi"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Syros"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Thessaloniki"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Tilos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Tinos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Volos"),
      -- TRANSLATORS: This Atlantean port is named after a harbour in Greece
      pgettext("portname", "Zakynthos"),
   }
}

pop_textdomain()
