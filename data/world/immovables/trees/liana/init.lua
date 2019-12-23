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
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "liana_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Liana Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_pole" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=30",
         "grow=liana_wasteland_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
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
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "liana_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Liana Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Liana Tree",
   icon = dirname .. "old/idle_0.png",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree", "normal_tree" },
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
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}
