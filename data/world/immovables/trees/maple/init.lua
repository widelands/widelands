dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 550,
   preferred_fertility = 800,
   pickiness = 80,
}

world:new_immovable_type{
   name = "maple_winter_sapling",
   descname = _ "Maple (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 57500",
         "remove=21",
         "grow=maple_winter_pole",
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
   name = "maple_winter_pole",
   descname = _ "Maple (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 52500",
         "remove=19",
         "grow=maple_winter_mature",
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
   name = "maple_winter_mature",
   descname = _ "Maple (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=maple_winter_old",
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
         hotspot = { 19, 49 }
      }
   },
}

world:new_immovable_type{
   name = "maple_winter_old",
   descname = _ "Maple (Old)",
   species = _ "Maple",
   icon = dirname .. "menu.png",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 39",
         "seed=maple_winter_sapling 240",
      },
      fall = {
         "remove=",
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
         hotspot = { 24, 60 },
         sound_effect = {
            path = "sound/animals/bird4",
         },
      },
   },
}
