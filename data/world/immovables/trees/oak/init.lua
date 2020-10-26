push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 90,
   preferred_humidity = 700,
   preferred_fertility = 500,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "oak_summer_sapling",
   descname = _ "Oak (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m12s500ms",
         "remove=chance:31.25%",
         "grow=oak_summer_pole",
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
   name = "oak_summer_pole",
   descname = _ "Oak (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m12s500ms",
         "remove=chance:27.34%",
         "grow=oak_summer_mature",
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
   name = "oak_summer_mature",
   descname = _ "Oak (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m15s",
         "remove=chance:15.62%",
         "grow=oak_summer_old",
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
         hotspot = { 18, 48 }
      }
   },
}

wl.Descriptions():new_immovable_type{
   name = "oak_summer_old",
   descname = _ "Oak (Old)",
   species = _ "Oak",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:37m30s",
         "transform=deadtree2 chance:4.69%",
         "seed=oak_summer_sapling proximity:39%",
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
         hotspot = { 24, 60 },
         sound_effect = {
            path = "sound/animals/bird2",
            priority = 10
         },
      },
      falling = {
         directory = dirname,
         basename = "falling",
         fps = 10,
         frames = 7,
         rows = 4,
         columns = 2,
         hotspot = { 10, 60 },
         play_once = true
      }
   },
}

pop_textdomain()
