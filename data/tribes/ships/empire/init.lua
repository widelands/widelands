push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ship_type {
   name = "empire_ship",
   -- TRANSLATORS: This is the Empire's ship's name used in lists of units
   descname = pgettext("empire_ship", "Ship"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,

   spritesheets = {
      idle = {
         fps = 10,
         frames = 40,
         rows = 7,
         columns = 6,
         hotspot = { 88, 90 }
      },
      sail = {
         fps = 10,
         frames = 40,
         rows = 7,
         columns = 6,
         directional = true,
         hotspot = { 111, 92 }
      },
   },

   names = {
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Adriatic Sea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Alexandria"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Amalea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Amolius"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Apollo"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Augustus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Azurea Sea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Baltic Sea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Bellona"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Ben Hur"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Bounty"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Brutus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Caligula"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Camilla"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Carrara"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Caesar"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Cicero"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Claudius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Diana"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Domitianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Empire Glory"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Faustulus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Galba"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Gallus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Ianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Iulius Caesar"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Iuno"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Iupiter"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Latium"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Latonia"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Leonardo"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Liguria"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Lutius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Marble"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Mars"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Minerva"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Neptune"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Nero"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Nerva"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Octavianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Otho"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Pluto"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Portus Bellus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Quirinus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Remus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Romulus"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Saledus"),
      -- TRANSLATORS: This Empire ship is named after a Roman deity
      pgettext("shipname", "Saturn"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Siena"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Sparta"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Thyrrenia"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Tiberius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Titus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Traianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Venus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vespasianus"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Vesta"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vinea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vitellius"),
   }
}

pop_textdomain()
