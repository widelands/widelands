dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 90,
   preferred_humidity = 0.7,
   preferred_fertility = 0.5,
   pickiness = 0.6,
}

world:new_immovable_type{
   name = "oak_summer_sapling",
   descname = _ "Oak (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72500",
         "remove=80",
         "grow=oak_summer_pole",
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
   name = "oak_summer_pole",
   descname = _ "Oak (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72500",
         "remove=70",
         "grow=oak_summer_mature",
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
   name = "oak_summer_mature",
   descname = _ "Oak (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 75000",
         "remove=40",
         "grow=oak_summer_old",
      },
   },
   animations = {
      idle = {
         template = "idle_?",
         directory = dirname .. "mature/",
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "oak_summer_old",
   descname = _ "Oak (Old)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 2250000",
         "transform=deadtree2 12",
         "seed=oak_summer_sapling",
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
         hotspot = { 24, 60 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird2",
         },
      },
      falling = {
         template = "f_tree_??",
         directory = dirname .. "old/",
         hotspot = { 10, 60 },
         fps = 10,
         play_once = true
      },
   },
}
