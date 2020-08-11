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
         "animate=idle duration:55s",
         "remove=chance:16.41%",
         "grow=spruce_summer_pole",
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
         hotspot = { 5, 13 }
      }
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
         "animate=idle duration:55s",
         "remove=chance:12.89%",
         "grow=spruce_summer_mature",
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
         hotspot = { 10, 29 }
      }
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
         "animate=idle duration:1m",
         "remove=chance:8.98%",
         "grow=spruce_summer_old",
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
         hotspot = { 13, 49 }
      }
   },
}

world:new_immovable_type{
   name = "spruce_summer_old",
   descname = _ "Spruce (Old)",
   species = _ "Spruce",
   icon = dirname .. "menu.png",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle duration:25m50s",
         "transform=deadtree3 chance:9.37%",
         "seed=spruce_summer_sapling 200",
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
         hotspot = { 16, 60 },
         sound_effect = {
            path = "sound/animals/bird3",
         },
      }
   },
}
