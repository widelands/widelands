push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 180,
   preferred_humidity = 400,
   preferred_fertility = 400,
   pickiness = 60,
}

wl.World():new_immovable_type{
   name = "palm_borassus_desert_sapling",
   descname = _ "Borassus Palm (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m10s",
         "remove=chance:31.25%",
         "grow=palm_borassus_desert_pole",
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

wl.World():new_immovable_type{
   name = "palm_borassus_desert_pole",
   descname = _ "Borassus Palm (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m10s",
         "remove=chance:27.34%",
         "grow=palm_borassus_desert_mature",
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

wl.World():new_immovable_type{
   name = "palm_borassus_desert_mature",
   descname = _ "Borassus Palm (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m5s",
         "remove=chance:15.62%",
         "grow=palm_borassus_desert_old",
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

wl.World():new_immovable_type{
   name = "palm_borassus_desert_old",
   descname = _ "Borassus Palm (Old)",
   species = _ "Borassus Palm",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:33m20s",
         "transform=deadtree5 chance:9.76%",
         "seed=palm_borassus_desert_sapling proximity:62.5%",
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
            path = "sound/animals/crickets1",
            priority = 10
         },
      }
   },
}

pop_textdomain()
