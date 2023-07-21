push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_ship_type {
   name = "frisians_ship",
   -- TRANSLATORS: This is the Frisians' ship's name used in lists of units
   descname = pgettext("frisians_ship", "Ship"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,

   hitpoints    = 50000,
   min_attack   =   100,
   max_attack   = 10000,
   defense         =  5,
   attack_accuracy = 60,
   heal_per_second = 100,

   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {41, 142}
      },
      sinking = {
         fps = 7,
         frames = 21,
         columns = 7,
         rows = 3,
         hotspot = {88, 141}
      },
      sail = {
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         directional = true,
         hotspot = {146, 153}
      },

      warship = {
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {58, 142}
      },
      sinking_warship = {
         fps = 7,
         frames = 21,
         columns = 7,
         rows = 3,
         hotspot = {88, 141}
      },
      sail_warship = {
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         directional = true,
         hotspot = {146, 153}
      },
   },

   names = {
      -- TRANSLATORS: This Frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Amrum"),
      -- TRANSLATORS: This Frisian ship is named after a no longer existing island in Northern Frisia
      pgettext("shipname", "Alt-Nordstrand"),
      -- TRANSLATORS: This Frisian ship is named after a peninsula in Northern Frisia
      pgettext("shipname", "Eiderstedt"),
      -- TRANSLATORS: This Frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Föhr"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Gröde"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Habel"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Hamburger Hallig"),
      -- TRANSLATORS: This Frisian ship is named after an island in the North Sea
      pgettext("shipname", "Helgoland"),
      -- TRANSLATORS: This Frisian ship is named after a region in Northern Frisia
      pgettext("shipname", "Hever"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Hooge"),
      -- TRANSLATORS: This Frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Japsand"),
      -- TRANSLATORS: This Frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Kormoransand"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Langeneß"),
      -- TRANSLATORS: This Frisian ship is named after a water body in Northern Frisia
      pgettext("shipname", "Norderaue"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Norderoog"),
      -- TRANSLATORS: This Frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Norderoogsand"),
      -- TRANSLATORS: This Frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Nordstrand"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Nordstrandischmoor"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Oland"),
      -- TRANSLATORS: This Frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Pellworm"),
      -- TRANSLATORS: This Frisian ship is named after a no longer existing island in Northern Frisia
      pgettext("shipname", "Strand"),
      -- TRANSLATORS: This Frisian ship is named after a water body in Northern Frisia
      pgettext("shipname", "Süderaue"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Süderoog"),
      -- TRANSLATORS: This Frisian ship is named after a sand in Northern Frisia
      pgettext("shipname", "Süderoogsand"),
      -- TRANSLATORS: This Frisian ship is named after a Hallig in Northern Frisia
      pgettext("shipname", "Südfall"),
      -- TRANSLATORS: This Frisian ship is named after an island in Northern Frisia
      pgettext("shipname", "Sylt"),
      -- TRANSLATORS: This Frisian ship is named after a region in Northern Frisia
      pgettext("shipname", "Uthlande"),
   }
}

pop_textdomain()
