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
   name = "birch_summer_pole",
   descname = _ "Birch (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 40000",
         "remove=25",
         "grow=birch_summer_mature",
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
         "seed=birch_summer_sapling 200",
         "animate=idle 30000",
         "remove=10",
         "grow=birch_summer_old",
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
   name = "birch_summer_old",
   descname = _ "Birch (Old)",
   species = _ "Birch",
   icon = dirname .. "menu.png",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 800000",
         "transform=deadtree2 27",
         "seed=birch_summer_sapling 60",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         directory = path.dirname(__file__),
         basename = "old",
         fps = 10,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 24, 59 },
         sound_effect = {
            path = "sound/animals/bird5",
         },
      }
   },
}
