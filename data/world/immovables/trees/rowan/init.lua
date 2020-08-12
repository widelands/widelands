dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 105,
   preferred_humidity = 650,
   preferred_fertility = 750,
   pickiness = 80,
}

world:new_immovable_type{
   name = "rowan_summer_sapling",
   descname = _ "Rowan (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "remove=chance:15.62%",
         "grow=rowan_summer_pole",
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
   name = "rowan_summer_pole",
   descname = _ "Rowan (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:11.72%",
         "grow=rowan_summer_mature",
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
   name = "rowan_summer_mature",
   descname = _ "Rowan (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:2.73%",
         "seed=rowan_summer_sapling 40",
         "animate=idle duration:30s",
         "remove=chance:3.91%",
         "grow=rowan_summer_old",
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
   name = "rowan_summer_old",
   descname = _ "Rowan (Old)",
   species = _ "Rowan",
   icon = dirname .. "menu.png",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:23m52s",
         "transform=deadtree4 chance:10.16%",
         "seed=rowan_summer_sapling 180",
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
            path = "sound/animals/bird6",
            priority = 10
         },
      }
   },
}
