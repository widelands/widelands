dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 0.2,
   preferred_fertility = 0.85,
   pickiness = 0.8,
}

world:new_immovable_type{
   name = "umbrella_green_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Umbrella Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 57000",
         "remove=21",
         "grow=umbrella_green_wasteland_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "sapling/", "idle_\\d+.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "umbrella_green_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Umbrella Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 53000",
         "remove=19",
         "grow=umbrella_green_wasteland_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "pole/", "idle_\\d+.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "umbrella_green_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Umbrella Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=umbrella_green_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "mature/", "idle_\\d+.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "umbrella_green_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Umbrella Tree (Old)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 39",
         "seed=umbrella_green_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}
