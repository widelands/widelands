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
   },

   port_names = {
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Aalkersem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Äfkebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ämelsbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ārichsem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Baamst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Beergem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bousbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bräist"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bualigsem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Deesbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Doogebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Emst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Faalst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Gaarding"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Guating"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Haatst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Hoorbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Hoorst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Hörnem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Hüsem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Iibel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kååting"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kairem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Klangsbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kläntem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Klasbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kuurlem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Madlem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Mälst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Mönkebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Moosbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Muasem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Naibel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Neebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Noorst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Olersem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Odenbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ödersem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Popenbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Raantem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Reinsbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Rikesbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Risem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Seebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Snootebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Strükem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Tååting"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Taning"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Teedenbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Tinem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Toorpem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ülwelsbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Wiisem"),
   }
}

pop_textdomain()
