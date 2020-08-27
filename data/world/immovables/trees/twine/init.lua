dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 95,
   preferred_humidity = 200,
   preferred_fertility = 400,
   pickiness = 80,
}

wl.World():new_immovable_type{
   name = "twine_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:16.41%",
         "grow=twine_wasteland_pole",
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
         hotspot = { 6, 13 }
      }
   },
}

wl.World():new_immovable_type{
   name = "twine_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:12.89%",
         "grow=twine_wasteland_mature",
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
         hotspot = { 13, 29 }
      }
   },
}

wl.World():new_immovable_type{
   name = "twine_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "remove=chance:8.98%",
         "grow=twine_wasteland_old",
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
         hotspot = { 19, 49 }
      }
   },
}

wl.World():new_immovable_type{
   name = "twine_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Twine Tree",
   icon = dirname .. "menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree3 chance:14.06%",
         "seed=twine_wasteland_sapling proximity:7.8%",
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
         hotspot = { 25, 61 }
      }
   },
}
