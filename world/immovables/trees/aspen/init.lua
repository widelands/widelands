dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 100,
   preferred_humidity = 0.6,
   preferred_fertility = 0.7,
   pickiness = 0.8,
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
         template = "idle_?",
         directory = dirname .. "sapling/",
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
         template = "idle_?",
         directory = dirname .. "pole/",
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
         template = "idle_?",
         directory = dirname .. "mature/",
         hotspot = { 18, 47 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "aspen_summer_old",
   descname = _ "Aspen (Old)",
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
         template = "idle_?",
         directory = dirname .. "old/",
         hotspot = { 23, 58 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird1",
         },
      },
      falling = {
         template = "f_tree_??",
         directory = dirname .. "old/",
         hotspot = { 20, 59 },
         fps = 10,
         play_once = true
      },
   },
}
