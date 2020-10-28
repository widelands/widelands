push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 150,
   preferred_fertility = 950,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "cirrus_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Cirrus Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m1s",
         "remove=chance:17.19%",
         "grow=cirrus_wasteland_pole",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "sapling",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 5, 12 }
      }
   },
}

wl.Descriptions():new_immovable_type{
   name = "cirrus_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Cirrus Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:59s",
         "remove=chance:13.28%",
         "grow=cirrus_wasteland_mature",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "pole",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 12, 28 }
      }
   },
}

wl.Descriptions():new_immovable_type{
   name = "cirrus_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Cirrus Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:9.37%",
         "grow=cirrus_wasteland_old",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "mature",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 18, 48 }
      }
   },
}

wl.Descriptions():new_immovable_type{
   name = "cirrus_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Cirrus Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Cirrus Tree",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:24m15s",
         "transform=deadtree3 chance:13.28%",
         "seed=cirrus_wasteland_sapling proximity:39%",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 10,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 24, 60 }
      }
   },
}

pop_textdomain()
