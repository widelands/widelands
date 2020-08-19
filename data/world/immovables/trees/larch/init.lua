dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 50,
   preferred_humidity = 800,
   preferred_fertility = 450,
   pickiness = 80,
}

world:new_immovable_type{
   name = "larch_summer_sapling",
   descname = _ "Larch (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "remove=chance:17.19%",
         "grow=larch_summer_pole",
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
   name = "larch_summer_pole",
   descname = _ "Larch (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "remove=chance:13.28%",
         "grow=larch_summer_mature",
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
   name = "larch_summer_mature",
   descname = _ "Larch (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:9.37%",
         "grow=larch_summer_old",
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
   name = "larch_summer_old",
   descname = _ "Larch (Old)",
   species = _ "Larch",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:24m15s",
         "transform=deadtree3 chance:8.98%",
         "seed=larch_summer_sapling proximity:11.72%",
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
            path = "sound/animals/bird6",
            priority = 10
         },
      }
   },
}
