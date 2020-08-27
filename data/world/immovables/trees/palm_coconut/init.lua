dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 130,
   preferred_humidity = 500,
   preferred_fertility = 600,
   pickiness = 60,
}

wl.World():new_immovable_type{
   name = "palm_coconut_desert_sapling",
   descname = _ "Coconut Palm (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:16.41%",
         "grow=palm_coconut_desert_pole",
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

wl.World():new_immovable_type{
   name = "palm_coconut_desert_pole",
   descname = _ "Coconut Palm (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:12.89%",
         "grow=palm_coconut_desert_mature",
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

wl.World():new_immovable_type{
   name = "palm_coconut_desert_mature",
   descname = _ "Coconut Palm (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "remove=chance:8.98%",
         "grow=palm_coconut_desert_old",
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

wl.World():new_immovable_type{
   name = "palm_coconut_desert_old",
   descname = _ "Coconut Palm (Old)",
   species = _ "Coconut Palm",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree6 chance:14.06%",
         "seed=palm_coconut_desert_sapling proximity:39%",
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
         hotspot = { 25, 60 },
         sound_effect = {
            path = "sound/animals/bird3",
            priority = 10
         },
      }
   },
}
