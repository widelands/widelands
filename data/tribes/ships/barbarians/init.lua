push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ship_type {
   name = "barbarians_ship",
   -- TRANSLATORS: This is the Barbarians' ship's name used in lists of units
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
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Boldreth"),
      -- TRANSLATORS: This Barbarian ship is named after an in-game character
      pgettext("shipname", "Chat’Karuth"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Dellingr"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Fulla"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Gersemi"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Hagbard"),
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
      pgettext("shipname", "Wolverine"),
      -- TRANSLATORS: This a Barbarian ship name
      pgettext("shipname", "Yrsa"),
   },
   port_names = {
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Åfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ål"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ålesund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Alstahaug"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Alta"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Alvdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Alver"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Åmli"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Åmot"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Andøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Aremark"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Arendal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Asker"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Årdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ås"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Åseral"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Askvoll"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Askøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Åsnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Aukra"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Aure"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Aurland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Aurskog-Høland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Austevoll"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Austrheim"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Averøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bærum"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Balsfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bamble"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bardu"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Båtsfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Beiarn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bergen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Berlevåg"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bindal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Birkenes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bjerkreim"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bjørnafjorden"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bø"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bodø"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bokn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bømlo"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bremanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Brønnøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bygland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Bykle"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Dønna"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Dovre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Drammen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Drangedal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Dyrøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Eidfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Eidskog"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Eidsvoll"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Eigersund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Elverum"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Enebakk"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Engerdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Etne"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Etnedal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Evenes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Evje"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Færder"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Farsund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Fauske"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Fedje"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Fitjar"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Fjaler"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Fjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Flå"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Flakstad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Flatanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Flekkefjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Flesberg"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Folldal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Fredrikstad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Frogn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Froland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Frosta"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Frøya"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Fyresdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gamvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gausdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gildeskål"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Giske"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gjemnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gjerdrum"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gjerstad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gjesdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gjøvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gloppen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gol"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gran"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Grane"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gratangen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Grimstad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Grong"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Grue"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Gulen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hå"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hadsel"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hægebostad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Halden"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hamar"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hamarøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hammerfest"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hareid"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Harstad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hasvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hattfjelldal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Haugesund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Heim"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hemnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hemsedal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Herøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Herøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hitra"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hjartdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hjelmeland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hol"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hole"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Holmestrand"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Holtålen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Horten"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Høyanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Høylandet"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hurdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hustadvika"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hvaler"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Hyllestad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ibestad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Inderøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Indre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Indre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Iveland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Jevnaker"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kåfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Karasjok"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Karlsøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Karmøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kautokeino"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kinn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Klepp"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kongsberg"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kongsvinger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kragerø"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kristiansand"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kristiansund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Krødsherad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kvæfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kvænangen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kvam"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kvinesdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kvinnherad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kviteseid"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Kvitsøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lærdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Larvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lavangen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lebesby"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Leirfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Leka"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lesja"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Levanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lier"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lierne"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lillehammer"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lillesand"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lillestrøm"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lindesnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lødingen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lom"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Loppa"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lørenskog"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Løten"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lunner"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lurøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Luster"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lyngdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Lyngen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Målselv"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Malvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Marker"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Masfjorden"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Måsøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Melhus"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Meløy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Meråker"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Midtre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Midt-Telemark"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Modalen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Modum"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Molde"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Moskenes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Moss"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nærøysund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Namsos"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Namsskogan"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nannestad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Narvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nesbyen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nesna"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nesodden"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nesseby"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nissedal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nittedal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nome"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nord-Aurdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nord-Fron"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nordkapp"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nord-Odal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nordreisa"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nordre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nordre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Nore"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Notodden"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Øksnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Oppdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Orkland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ørland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ørsta"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Osen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Os"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Oslo"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Osterøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Østre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Overhalla"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Øvre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Øyer"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Øygarden"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Øystre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Porsanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Porsgrunn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Råde"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rælingen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rakkestad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rana"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Randaberg"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rauma"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rendalen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rennebu"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rindal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ringebu"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ringerike"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ringsaker"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Risør"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rødøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Rollag"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Røros"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Røst"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Røyrvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Salangen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Saltdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Samnanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sandefjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sande"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sandnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sarpsborg"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sauda"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Selbu"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Seljord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sel"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Senja"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sigdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Siljan"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sirdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Skaun"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Skien"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Skiptvet"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Skjåk"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Skjervøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Smøla"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Snåsa"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sogndal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sokndal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sola"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Solund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sømna"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Søndre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sør-Aurdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sørfold"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sør-Fron"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sør-Odal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sørreisa"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sortland"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sør-Varanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stange"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stavanger"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Steigen"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Steinkjer"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stjørdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stor-Elvdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Storfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stranda"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Strand"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Stryn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sula"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Suldal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sunndal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sunnfjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Surnadal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sveio"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Sykkylven"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tana"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Time"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tingvoll"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tinn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tjeldsund"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tokke"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tolga"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tønsberg"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Træna"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tromsø"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Trondheim"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Trysil"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tvedestrand"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tydal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tynset"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tysnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Tysvær"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ullensaker"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ullensvang"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ulstein"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Ulvik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Utsira"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vadsø"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Værøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vågå"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vågan"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vaksdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Våler"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Våler"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Valle"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vang"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vanylven"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vardø"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vefsn"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vega"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vegårshei"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vennesla"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Verdal"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vestby"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vestnes"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vestre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vestre"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vestvågøy"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vevelstad"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vik"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vindafjord"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Vinje"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Volda"),
      -- TRANSLATORS: This Barbarian port is named after a Norwegian town
      pgettext("portname", "Voss"),
   }
}

pop_textdomain()
