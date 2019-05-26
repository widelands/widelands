dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 100,
   preferred_humidity = 600,
   preferred_fertility = 700,
   pickiness = 80,
}

world:new_immovable_type{
   name = "aspen_summer_sapling",
   descname = _ "Aspen (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 38000",
         "remove=50",
         "grow=aspen_summer_pole",
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
   name = "aspen_summer_pole",
   descname = _ "Aspen (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 38000",
         "remove=47",
         "grow=aspen_summer_mature",
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
   name = "aspen_summer_mature",
   descname = _ "Aspen (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 20000",
         "remove=30",
         "seed=aspen_summer_sapling",
         "animate=idle 20000",
         "remove=20",
         "grow=aspen_summer_old",
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
   name = "aspen_summer_old",
   descname = _ "Aspen (Old)",
   species = _ "Aspen",
   icon = dirname .. "old/idle_0.png",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 15",
         "seed=aspen_summer_sapling",
      },
      fall = {
         "animate=falling 1400",
         "transform=fallentree",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 23, 58 },
         fps = 10,
         sound_effect = {
            path = "sound/animals/bird1",
         },
      },
      falling = {
         pictures = path.list_files(dirname .. "old/f_tree_??.png"),
         hotspot = { 20, 59 },
         fps = 10,
         play_once = true
      },
   },
}
