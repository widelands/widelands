dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 100,
   preferred_humidity = 200,
   preferred_fertility = 800,
   pickiness = 80,
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m12s500ms",
         "remove=chance:31.25%",
         "grow=mushroom_dark_wasteland_pole",
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

world:new_immovable_type{
   name = "mushroom_dark_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m12s500ms",
         "remove=chance:27.34%",
         "grow=mushroom_dark_wasteland_mature",
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

world:new_immovable_type{
   name = "mushroom_dark_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m15s",
         "remove=chance:15.62%",
         "grow=mushroom_dark_wasteland_old",
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

world:new_immovable_type{
   name = "mushroom_dark_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Dark Mushroom Tree",
   icon = dirname .. "menu.png",
   editor_category = "trees_wasteland",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:17m30s",
         "transform=deadtree2 chance:9.76%",
         "seed=mushroom_dark_wasteland_sapling proximity:78.12%",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 15,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 25, 61 }
      }
   },
}
