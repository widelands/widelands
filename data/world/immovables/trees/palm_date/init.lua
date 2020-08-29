dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 160,
   preferred_humidity = 500,
   preferred_fertility = 500,
   pickiness = 80,
}

wl.World():new_immovable_type{
   name = "palm_date_desert_sapling",
   descname = _ "Date Palm (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:38s",
         "remove=chance:19.53%",
         "grow=palm_date_desert_pole",
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
   name = "palm_date_desert_pole",
   descname = _ "Date Palm (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:38s",
         "remove=chance:18.36%",
         "grow=palm_date_desert_mature",
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
   name = "palm_date_desert_mature",
   descname = _ "Date Palm (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:11.72%",
         "seed=palm_date_desert_sapling proximity:7.8%",
         "animate=idle duration:20s",
         "remove=chance:7.81%",
         "grow=palm_date_desert_old",
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
   name = "palm_date_desert_old",
   descname = _ "Date Palm (Old)",
   species = _ "Date Palm",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:17m30s",
         "transform=deadtree5 chance:12.5%",
         "seed=palm_date_desert_sapling proximity:78.12%",
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
         hotspot = { 25, 61 },
         sound_effect = {
            path = "sound/animals/crickets1",
            priority = 10
         },
      }
   },
}
