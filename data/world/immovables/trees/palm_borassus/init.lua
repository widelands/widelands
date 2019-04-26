dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 180,
   preferred_humidity = 400,
   preferred_fertility = 400,
   pickiness = 60,
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
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
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
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
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
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "palm_borassus_desert_old",
   descname = _ "Borassus Palm (Old)",
   species = _ "Borassus Palm",
   icon = dirname .. "old/idle_0.png",
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
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
         sound_effect = {
            path = "sound/animals/crickets1",
         },
      },
   },
}
