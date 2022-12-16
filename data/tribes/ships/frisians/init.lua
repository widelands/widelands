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
      pgettext("portname", "Åktoorp"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Oornshaud"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Årnfjål"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Årnfjålfäil"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Aalkersem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Aalmtoorp"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Alwat"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Oowentoft"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Beergem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bjarntoorp"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bååmst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bonlem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Boorlem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Borigsem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bousbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Braarep"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bramstedtlund"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Bräist"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Brääklem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Doogebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Draage"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Trölstrup"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Dunsem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Elisabeth-Sophien-Kuuch"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ellhöft"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ämelsbel-Hoorbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ding-Sönj"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Freesendelf"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Friedrich-Wilhelm-Lübke-Kuuch"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Friedrichstääd"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Galmsbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Garding"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Sarkspal Garding"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Golbäk"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Gelün"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "A Grööd"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Grothusenkuuch"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "A Huug"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Haselund"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Haatst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Haatstinger Määrsch"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Höögel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Holm"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Hörnem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Hoorst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Huude"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Humptoorp"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Hüsem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Imensteed"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Jåålönj"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kaamp"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kuurlem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Katharinenheerd"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Klangsbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Klasbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Koolnbütel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kolkerhii"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Kotzenbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Låålönj"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "A Nääs"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "E Hoorne"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Leek"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Leeksguurd"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "List"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Jöömst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Läitjholem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Madlem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Melst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Neebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Naischöspel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Njiblem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Naibel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Noorsaarep"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Nuurderfriedrichskuuch"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Nuurdstrun"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Noorst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "E Hoolme"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ööwenem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Oldenswort"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Oldersbek"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Åldrop"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Olersem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Uastenfial"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Aaster-Uurst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Uasterheewer"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Pelwerem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Poppenbüll"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ramsteed"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Raantrem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Reußenkuuger"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Risem-Lonham"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Runees"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Sankt Peter-Ording"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Swåbstää"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Swiasing"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Seet"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Simonsberag"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Salwit"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Säänebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Språkebel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ståårem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Stääsönj"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Strükem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Söleraanj"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Süderhöft"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Läigem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Süüdermaask"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Söl’"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Tating"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Tetenbüll"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Taningstää"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Tönang"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Tümlauer Kuuch"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ülwesbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Äphüsem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Ödersem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Fjåål"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Folerwiik"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Fåålst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Wäilt"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Woningstair-Brēderep"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Weester-Uurst"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Waasterheewer"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Weestre"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Winert"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "E Wäsk"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Wiisem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Witbek"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Witjdün"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Witswort"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Wååbel"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "Wraksem"),
      -- TRANSLATORS: This Frisian port is named after a town in Northern Frisia
      pgettext("portname", "A Wik"),
   }
}

pop_textdomain()
