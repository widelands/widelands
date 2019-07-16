dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 95,
   preferred_humidity = 200,
   preferred_fertility = 400,
   pickiness = 80,
}

world:new_immovable_type{
   name = "twine_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=42",
         "grow=twine_wasteland_pole",
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
   name = "twine_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=33",
         "grow=twine_wasteland_mature",
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
   name = "twine_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=23",
         "grow=twine_wasteland_old",
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
   name = "twine_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Twine Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Twine Tree",
   icon = dirname .. "old/idle_0.png",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree3 36",
         "seed=twine_wasteland_sapling",
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
