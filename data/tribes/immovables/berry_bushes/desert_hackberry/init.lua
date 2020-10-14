push_textdomain("tribes")

dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 170, -- Temperature is in arbitrary units.
   preferred_humidity = 50,     -- Values between 0 and 1000 (1000 being very wet).
   preferred_fertility = 50,    -- Values between 0 and 1000 (1000 being very fertile).
   pickiness = 30,             -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   name = "berry_bush_desert_hackberry_tiny",
   descname = _ "Desert Hackberry (tiny)",
   size = "small",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m20s",
         "grow=berry_bush_desert_hackberry_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {10, 9},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_desert_hackberry_small",
   descname = _ "Desert Hackberry (small)",
   size = "small",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "grow=berry_bush_desert_hackberry_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {19, 21},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_desert_hackberry_medium",
   descname = _ "Desert Hackberry (medium)",
   size = "small",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "grow=berry_bush_desert_hackberry_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {21, 27},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_desert_hackberry_ripe",
   descname = _ "Desert Hackberry (ripe)",
   size = "small",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "ripe",
         basename = "idle",
         hotspot = {21, 27},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
