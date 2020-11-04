push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 100,
   preferred_humidity = 600,
   preferred_fertility = 700,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "aspen_summer_sapling",
   descname = _ "Aspen (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:38s",
         "remove=chance:19.53%",
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
         hotspot = { 5, 12 }
      }
   },
}

wl.Descriptions():new_immovable_type{
   name = "aspen_summer_pole",
   descname = _ "Aspen (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:38s",
         "remove=chance:18.36%",
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
         hotspot = { 12, 28 }
      }
   },
}

wl.Descriptions():new_immovable_type{
   name = "aspen_summer_mature",
   descname = _ "Aspen (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:11.72%",
         "seed=aspen_summer_sapling proximity:23.44%",
         "animate=idle duration:20s",
         "remove=chance:7.81%",
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
         hotspot = { 18, 47 }
      }
   },
}

wl.Descriptions():new_immovable_type{
   name = "aspen_summer_old",
   descname = _ "Aspen (Old)",
   species = _ "Aspen",
   icon = dirname .. "menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:17m30s",
         "transform=deadtree2 chance:5.86%",
         "seed=aspen_summer_sapling proximity:39%",
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
         hotspot = { 23, 58 }
      },
      falling = {
         directory = dirname,
         basename = "falling",
         fps = 10,
         frames = 9,
         rows = 3,
         columns = 3,
         hotspot = { 20, 59 },
         play_once = true
      }
   },
}

pop_textdomain()
