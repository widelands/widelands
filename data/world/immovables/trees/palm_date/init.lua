dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 311.6086872522219,
   preferred_humidity = 0.55977490370368566,
   preferred_fertility = 0.24221393314943546,
   pickiness = 0.53647728188312771,
}

world:new_immovable_type{
   name = "palm_date_desert_sapling",
   descname = _ "Date Palm (Sapling)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 38000",
         "remove=50",
         "grow=palm_date_desert_pole",
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
   name = "palm_date_desert_pole",
   descname = _ "Date Palm (Pole)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 38000",
         "remove=47",
         "grow=palm_date_desert_mature",
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
   name = "palm_date_desert_mature",
   descname = _ "Date Palm (Mature)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 20000",
         "remove=30",
         "seed=palm_date_desert_sapling",
         "animate=idle 20000",
         "remove=20",
         "grow=palm_date_desert_old",
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
   name = "palm_date_desert_old",
   descname = _ "Date Palm (Old)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree5 32",
         "seed=palm_date_desert_sapling",
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
         sound_effect = {
            directory = "animals",
            name = "bird1",
         },
      },
   },
}
