dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 90,
   preferred_humidity = 700,
   preferred_fertility = 500,
   pickiness = 60,
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
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
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
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
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
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "oak_summer_old",
   descname = _ "Oak (Old)",
   species = _ "Oak",
   icon = dirname .. "old/idle_0.png",
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
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
         sound_effect = {
            path = "sound/animals/bird2",
         },
      },
      falling = {
         pictures = path.list_files(dirname .. "old/f_tree_??.png"),
         hotspot = { 10, 60 },
         fps = 10,
         play_once = true
      },
   },
}
