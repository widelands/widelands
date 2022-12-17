push_textdomain("tribes")

dirname = path.dirname(__file__)

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
      pgettext("shipname", "Hakhor")
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
   },
   port_names = {
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Al’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Al’sund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Al’thaug"),
      -- TRANSLATORS: This Barbarian port is named after the barbarian capital.
      pgettext("portname", "Al’thunran"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Al’ta"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Al’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Al’ver"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "An’dey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ar’mark"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ar’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Asker"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "As’ral"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "As’kvoll"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "As’key"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "As’nes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Au’kra"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Au’re"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Au’land"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Au’skog"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Au’tevoll"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Au’trheim"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Av’rey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bærum"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bal’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ba’ble"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ba’rdu"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bat’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "B’iarn"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Berl’vog"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bindal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Birk’nes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bjer’kreim"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bjer’fjord’n"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bo’do"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bok’n"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bom’lo"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bremanger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Bren’ey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "B’gland"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "B’kle"),
      -- TRANSLATORS: This Barbarian port is named after a barbarian town.
      pgettext("portname", "Damack"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Danna"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Do’vre"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "D’rammen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "D’rangedal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "D’rey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ei’dfjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ei’dskog"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ei’dsvoll"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ei’g’rsund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Elv’rum"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "En’bakk"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Eng’rdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Et’ne"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Et’nedal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ev’nes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ev’je"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Eyer"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Eygard’n"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Eystre"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Færdr"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Farsund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Foske"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "F’dje"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "F’tjar"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "F’jaler"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Flo"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Flakstad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Flatang’r"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Flek’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Fl’sberg"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Folldal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Frogn"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Fr’land"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Frosta"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Freya"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Fyr’sdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gamvik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gausdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gild’skol"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gi’ske"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "G’jemnes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gjerdrum"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gjerst’d"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "G’jesdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gjovik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Glopp’n"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gol"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gran"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gra’ne"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Gr’tangen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Grimst’d"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Grong"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Grue"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "G’len"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ho"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’dsel"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hæg’bost’d"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’lden"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’mar"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’marey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’merfest"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’reid"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’rstad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hasvik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hattfjelldal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Haug’sund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Heim"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’mnes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hems’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’rey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’roy"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hitra"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hjartdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hjelm’land"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hol"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hole"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Holm’strand"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hol’talen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Horten"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’yanger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’ylandet"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’rdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hustadvik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "H’valer"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Hyll’stad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ib’stad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ind’rey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ind’re"),
      -- TRANSLATORS: This Barbarian port is named after a barbarian town.
      pgettext("portname", "Ir Kal’tur"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Iv’land"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "J’vnaker"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "K’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kar’sjok"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Karls’y"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Karm’y"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kot’keino"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "K’nn"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kl’pp"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "K’ngsberg"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "K’ngsvinger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Krag’re"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kr’dsherad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kvæfjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kvæn’ngen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kvam"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kvin’sdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kvin’herad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kvit’seid"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Kvit’sey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lærdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Larvik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lav’ngen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’besby"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’rfjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’ka"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’sja"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’vanger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lier"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’rne"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lil’hammer"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lil’sand"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lil’strom"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lind’snes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’dingen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lom"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Loppa"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lar’nskog"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’ten"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Lunner"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’rey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’ster"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’ngdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "L’ngen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mal’selv"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mal’vik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mar’ker"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mas’fjordn"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mos’y"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "M’lhus"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "M’ley"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mer’ker"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Midt’re"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Midt’lemark"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mod’al’en"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Modum"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "M’lde"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mos’kenes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Mos’s"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Næreysund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Namsos"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nams’skogan"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nan’stad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nar’vik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "N’sbyen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nesna"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nesod’n"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nes’by"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nis’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nit’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nome"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nord’aurdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nord’fron"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nord’kapp"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nord’odal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nord’reisa"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Nord’re"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "No’re"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Notod’n"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ok’snes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Omli"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Omot"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Op’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Or’kland"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Or’land"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Or’sta"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Osen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Os"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Os’lo"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ost’rey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ost’re"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ov’rhala"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ov’re"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Pors’nger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Pors’grun"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rode"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ræl’ngen"),
      -- TRANSLATORS: This Barbarian port is named after a barbarian town.
      pgettext("portname", "Rænt"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rak’stad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rana"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rand’berg"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rauma"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ren’dlen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ren’bu"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rin’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rin’gbu"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rin’grike"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rin’gsker"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ris’r"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "R’dey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rol’lag"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "R’ros"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Rest"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Reyr’vik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sal’ngen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sal’tdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Samn’nger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sand’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sand’e"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sand’nes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sarpsb’rg"),
      -- TRANSLATORS: This Barbarian port is named after a barbarian town.
      pgettext("portname", "Sars’tun"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sauda"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sel’bu"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sel’jord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sel"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sen’ja"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sig’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sil’jan"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sir’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Skaun"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Skien"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Skipt’vet"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Skjok"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Skjer’vey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Smola"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Snosa"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sog’ndal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sok’ndal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sola"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sol’nd"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Som’na"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Son’dre"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sor’aur’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sor’fold"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sor’fron"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sor’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sor’reisa"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sor’tland"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sor’var’nger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stange"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stav’nger"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "St’gen"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "St’nkjer"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stjor’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stor’elvdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stor’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stranda"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Strand"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Stryn"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sula"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Suldal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sun’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sun’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Surn’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Sveio"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Syk’lven"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tana"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Thronsand"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Thronsund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Time"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "T’ngvol"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ti’n"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tjeld’sund"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Toke"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tolga"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tonsberg"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Træna"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tromso"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tr’ndheim"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tr’sil"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tved’strand"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "T’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tynset"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tys’nes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Tys’vær"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ul’nsaker"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ul’nsvang"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ul’stein"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ul’vik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Uts’ra"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vadso"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Værey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vogo"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’gan"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’ksdal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’ler"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vale"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vang"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Van’lven"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vardo"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vef’sn"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vega"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Veg’rshei"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ven’sla"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Ver’dal"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’stby"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’stnes"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’stre"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’stvogey"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vev’lstad"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vik"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vind’fjord"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vin’je"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "V’lda"),
      -- TRANSLATORS: This Barbarian port is inspired by a Norwegian town name.
      pgettext("portname", "Vos"),
   }
}

pop_textdomain()
