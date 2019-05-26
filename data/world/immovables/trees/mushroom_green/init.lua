dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 105,
   preferred_humidity = 200,
   preferred_fertility = 900,
   pickiness = 80,
}

world:new_immovable_type{
   name = "mushroom_green_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Mushroom Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=mushroom_green_wasteland_pole",
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
   name = "mushroom_green_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Mushroom Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=mushroom_green_wasteland_mature",
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
   name = "mushroom_green_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Mushroom Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=mushroom_green_wasteland_old",
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
   name = "mushroom_green_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Mushroom Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Green Mushroom Tree",
   icon = dirname .. "old/idle_0.png",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree2 33",
         "seed=mushroom_green_wasteland_sapling",
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
