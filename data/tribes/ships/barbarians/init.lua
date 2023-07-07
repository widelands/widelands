push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ship_type {
   name = "barbarians_ship",
   -- TRANSLATORS: This is the Barbarians’ ship’s name used in lists of units
   descname = pgettext("barbarians_ship", "Ship"),
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
         hotspot = { 72, 60 }
      },
      sail = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 103, 71 }
      },
   },

   names = {
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Agilaz"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Aslaug"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Baldr"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Bear"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Beowulf"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Bison"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Boldreth"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Chat’Karuth"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Dellingr"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Eagle"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Fulla"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Gersemi"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Hagbard"),
      -- TRANSLATORS: This Barbarian ship is named after a barbarian bard
      pgettext("shipname", "Hakhor"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Heidrek"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Heimdallr"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Hnoss"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Hrothgar"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Ingeld"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Karl Hundason"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Khantrukh"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Lynx"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Mani"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Odin"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Red Fox"),
      -- TRANSLATORS: This Barbarian ship is mentioned in some lore texts
      pgettext("shipname", "Saxnot"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Sigmund"),
      -- TRANSLATORS: This Barbarian ship is named after a barbarian bard
      pgettext("shipname", "Sigurd"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Snotra"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Thor"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Thron"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Ullr"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Valdar"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Vili"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Volf"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Wild Boar"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Valkyrie"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Wisent"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Wolverine"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Yrsa"),
   }
}

pop_textdomain()
