dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 120,
   preferred_humidity = 200,
   preferred_fertility = 700,
   pickiness = 60,
}

world:new_immovable_type{
   name = "liana_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Liana Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=40",
         "grow=liana_wasteland_pole",
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
   name = "liana_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Liana Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=30",
         "grow=liana_wasteland_mature",
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
   name = "liana_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Liana Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=10",
         "seed=liana_wasteland_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=liana_wasteland_old",
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
   name = "liana_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Liana Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Liana Tree",
   icon = dirname .. "menu.png",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree4 48",
         "seed=liana_wasteland_sapling",
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
