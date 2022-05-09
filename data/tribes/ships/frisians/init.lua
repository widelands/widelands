push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_ship_type {
   name = "frisians_ship",
   -- TRANSLATORS: This is the Frisians' ship's name used in lists of units
   descname = pgettext("frisians_ship", "Ship"),
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {79, 143}
      },
      sinking = {
         directory = dirname,
         basename = "sinking",
         fps = 7,
         frames = 21,
         columns = 7,
         rows = 3,
         hotspot = {88, 141}
      },
      sail = {
         directory = dirname,
         basename = "sail",
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         directional = true,
         hotspot = {146, 153}
      },
   },

   names = {
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Amrum"),
      -- TRANSLATORS: This frisian ship is named after a no longer existing island in Northern Frisia
      pgettext("shipname", "Alt-Nordstrand"),
      -- TRANSLATORS: This frisian ship is named after a peninsula in Northern Frisia
      pgettext("shipname", "Eiderstedt"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Föhr"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Gröde"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Habel"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Hamburger Hallig"),
      -- TRANSLATORS: This frisian ship is named after an island in the North Sea
      pgettext("shipname", "Helgoland"),
      -- TRANSLATORS: This frisian ship is named after a region in Northern Frisia
      pgettext("shipname", "Hever"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Hooge"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Japsand"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Kormoransand"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Langeneß"),
      -- TRANSLATORS: This frisian ship is named after a water body in Northern Frisia
      pgettext("shipname", "Norderaue"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Norderoog"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Norderoogsand"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Nordstrand"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Nordstrandischmoor"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Oland"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Pellworm"),
      -- TRANSLATORS: This frisian ship is named after a no longer existing island in Northern Frisia
      pgettext("shipname", "Strand"),
      -- TRANSLATORS: This frisian ship is named after a water body in Northern Frisia
      pgettext("shipname", "Süderaue"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Süderoog"),
      -- TRANSLATORS: This frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Süderoogsand"),
      -- TRANSLATORS: This frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Südfall"),
      -- TRANSLATORS: This frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Sylt"),
      -- TRANSLATORS: This frisian ship is named after a region in Northern Frisia
      pgettext("shipname", "Uthlande"),
   }
}

pop_textdomain()
