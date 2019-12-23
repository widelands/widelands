dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 80,
   preferred_humidity = 650,
   preferred_fertility = 600,
   pickiness = 60,
}

world:new_immovable_type{
   name = "birch_summer_sapling",
   descname = _ "Birch (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 42000",
         "remove=32",
         "grow=birch_summer_pole",
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
   name = "birch_summer_pole",
   descname = _ "Birch (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_pole" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 40000",
         "remove=25",
         "grow=birch_summer_mature",
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
   name = "birch_summer_mature",
   descname = _ "Birch (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 25000",
         "remove=10",
         "seed=birch_summer_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=birch_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 47 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "birch_summer_old",
   descname = _ "Birch (Old)",
   species = _ "Birch",
   icon = dirname .. "old/idle_0.png",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree", "normal_tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 800000",
         "transform=deadtree2 27",
         "seed=birch_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 23, 58 },
         fps = 10,
         sound_effect = {
            path = "sound/animals/bird5",
         },
      },
   },
}
