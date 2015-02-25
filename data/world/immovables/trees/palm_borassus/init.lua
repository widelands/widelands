dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 312.12514766707613,
   preferred_humidity = 0.56274510555621171,
   preferred_fertility = 0.24489562637623052,
   pickiness = 0.54267541248001672,
}

world:new_immovable_type{
   name = "palm_borassus_desert_sapling",
   descname = _ "Borassus Palm (Sapling)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 70000",
         "remove=80",
         "grow=palm_borassus_desert_pole",
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
   name = "palm_borassus_desert_pole",
   descname = _ "Borassus Palm (Pole)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 70000",
         "remove=70",
         "grow=palm_borassus_desert_mature",
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
   name = "palm_borassus_desert_mature",
   descname = _ "Borassus Palm (Mature)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=40",
         "grow=palm_borassus_desert_old",
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
   name = "palm_borassus_desert_old",
   descname = _ "Borassus Palm (Old)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 2000000",
         "transform=deadtree5 25",
         "seed=palm_borassus_desert_sapling",
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
            name = "bird2",
         },
      },
   },
}
