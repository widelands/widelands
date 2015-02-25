dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 300.3652506361999,
   preferred_humidity = 0.65223396993513105,
   preferred_fertility = 0.34545960943365628,
   pickiness = 0.59177154775649499,
}

world:new_immovable_type{
   name = "palm_oil_desert_sapling",
   descname = _ "Oil Palm (Sapling)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 42000",
         "remove=32",
         "grow=palm_oil_desert_pole",
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
   name = "palm_oil_desert_pole",
   descname = _ "Oil Palm (Pole)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 40000",
         "remove=25",
         "grow=palm_oil_desert_mature",
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
   name = "palm_oil_desert_mature",
   descname = _ "Oil Palm (Mature)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 25000",
         "remove=10",
         "seed=palm_oil_desert_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=palm_oil_desert_old",

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
   name = "palm_oil_desert_old",
   descname = _ "Oil Palm (Old)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 800000",
         "transform=deadtree5 50",
         "seed=palm_oil_desert_sapling",
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
            name = "bird5",
         },
      },
   },
}
