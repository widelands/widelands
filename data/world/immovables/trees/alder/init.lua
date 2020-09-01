push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   -- Temperature is in arbitrary units.
   preferred_temperature = 125,

   -- Value between 0 and 1000 (1000 being very wet).
   preferred_humidity = 650,

   -- Values between 0 and 1000 (1000 being very fertile).
   preferred_fertility = 600,

   -- A value in [0, 100] that defines how well this can deal with non-ideal
   -- situations. Lower means it is less picky, i.e. it can deal better.
   pickiness = 60,
}

wl.World():new_immovable_type{
   name = "alder_summer_sapling",
   descname = _ "Alder (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:57s500ms",
         "remove=chance:8.2%",
         "grow=alder_summer_pole",
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
   name = "alder_summer_pole",
   descname = _ "Alder (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:52s500ms",
         "remove=chance:7.42%",
         "grow=alder_summer_mature",
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
   name = "alder_summer_mature",
   descname = _ "Alder (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:50s",
         "remove=chance:7%",
         "grow=alder_summer_old",
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
   name = "alder_summer_old",
   descname = _ "Alder (Old)",
   species = _ "Alder",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree4 chance:1.95%",
         "seed=alder_summer_sapling proximity:70.31%",
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
            priority = 10
         }
      }
   },
}

pop_textdomain()
