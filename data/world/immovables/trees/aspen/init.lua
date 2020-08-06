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
         "animate=idle duration:38s",
         "remove=success:50",
         "grow=aspen_summer_pole",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "sapling",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 6, 13 }
      }
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
         "animate=idle duration:38s",
         "remove=success:47",
         "grow=aspen_summer_mature",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "pole",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 13, 29 }
      }
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
         "animate=idle duration:20s",
         "remove=success:30",
         "seed=aspen_summer_sapling 60",
         "animate=idle duration:20s",
         "remove=success:20",
         "grow=aspen_summer_old",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "mature",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 19, 48 }
      }
   },
}

world:new_immovable_type{
   name = "aspen_summer_old",
   descname = _ "Aspen (Old)",
   species = _ "Aspen",
   icon = dirname .. "menu.png",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle duration:17m30s",
         "transform=deadtree2 success:15",
         "seed=aspen_summer_sapling 100",
      },
      fall = {
         "animate=falling duration:1s400ms",
         "transform=fallentree",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 10,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 24, 59 }
      },
      falling = {
         directory = dirname,
         basename = "falling",
         fps = 10,
         frames = 9,
         rows = 3,
         columns = 3,
         hotspot = { 21, 60 },
         play_once = true
      }
   },
}
