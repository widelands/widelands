dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 105,
   preferred_humidity = 0.65,
   preferred_fertility = 0.75,
   pickiness = 0.8,
}

world:new_immovable_type{
   name = "rowan_summer_sapling",
   descname = _ "Rowan (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=40",
         "grow=rowan_summer_pole",
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
   name = "rowan_summer_pole",
   descname = _ "Rowan (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=30",
         "grow=rowan_summer_mature",
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
   name = "rowan_summer_mature",
   descname = _ "Rowan (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=7",
         "seed=rowan_summer_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=rowan_summer_old",
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
   name = "rowan_summer_old",
   descname = _ "Rowan (Old)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1432000",
         "transform=deadtree4 26",
         "seed=rowan_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         template = "idle_?",
         directory = dirname .. "old/",
         hotspot = { 23, 59 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird6",
         },
      },
   },
}
