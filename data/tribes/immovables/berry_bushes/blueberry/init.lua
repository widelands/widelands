push_textdomain("tribes")

dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 45, -- Temperature is in arbitrary units.
   preferred_humidity = 750,   -- Values between 0 and 1000 (1000 being very wet).
   preferred_fertility = 400,  -- Values between 0 and 1000 (1000 being very fertile).
   pickiness = 15,             -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   name = "berry_bush_blueberry_tiny",
   descname = _ "Blueberry Bush (tiny)",
   size = "small",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:18s",
         "grow=berry_bush_blueberry_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {6, 7},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_blueberry_small",
   descname = _ "Blueberry Bush (small)",
   size = "small",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:36s",
         "grow=berry_bush_blueberry_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {12, 13},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_blueberry_medium",
   descname = _ "Blueberry Bush (medium)",
   size = "small",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:24s",
         "grow=berry_bush_blueberry_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {15, 16},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_blueberry_ripe",
   descname = _ "Blueberry Bush (ripe)",
   size = "small",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:10m",
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
         hotspot = {15, 16},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}
pop_textdomain()
