sedirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 90,
   preferred_humidity = 150,
   preferred_fertility = 825,
   pickiness = 80,
}

world:new_immovable_type{
   name = "umbrella_red_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Umbrella Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 38000",
         "remove=50",
         "grow=umbrella_red_wasteland_pole",
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
   name = "umbrella_red_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Umbrella Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 38000",
         "remove=47",
         "grow=umbrella_red_wasteland_mature",
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
   name = "umbrella_red_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Umbrella Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 20000",
         "remove=30",
         "seed=umbrella_red_wasteland_sapling",
         "animate=idle 20000",
         "remove=20",
         "grow=umbrella_red_wasteland_old",
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
   name = "umbrella_red_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Umbrella Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Red Umbrella Tree",
   icon = dirname .. "old/idle_0.png",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 32",
         "seed=umbrella_red_wasteland_sapling",
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
