dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 60,
   preferred_humidity = 700,
   preferred_fertility = 500,
   pickiness = 60,
}

world:new_immovable_type{
   name = "spruce_summer_sapling",
   descname = _ "Spruce (Sapling)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=42",
         "grow=spruce_summer_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
         hotspot = { 4, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "spruce_summer_pole",
   descname = _ "Spruce (Pole)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=33",
         "grow=spruce_summer_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
         hotspot = { 9, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "spruce_summer_mature",
   descname = _ "Spruce (Mature)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=23",
         "grow=spruce_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 12, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "spruce_summer_old",
   descname = _ "Spruce (Old)",
   species = _ "Spruce",
   icon = dirname .. "old/idle_0.png",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree3 24",
         "seed=spruce_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 15, 59 },
         fps = 10,
         sound_effect = {
            path = "sound/animals/bird3",
         },
      },
   },
}
