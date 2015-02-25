dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 283.81248780942167,
   preferred_humidity = 0.70557470200287675,
   preferred_fertility = 0.33588939650946081,
   pickiness = 0.59843914180773927,
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
         pictures = path.list_directory(dirname .. "sapling/", "idle_\\d+.png"),
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
         pictures = path.list_directory(dirname .. "pole/", "idle_\\d+.png"),
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
         pictures = path.list_directory(dirname .. "mature/", "idle_\\d+.png"),
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
         "animate=falling 1200",
         "transform=fallentree",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 23, 58 },
         fps = 10,
         sound_effect = {
            directory = "animals",
            name = "bird1",
         },
      },
      falling = {
         pictures = path.list_directory(dirname .. "old/", "f_tree_\\d+.png"),
         hotspot = { 20, 59 },
         fps = 10,
      },
   },
}
