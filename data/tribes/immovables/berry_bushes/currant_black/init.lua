push_textdomain("tribes")

dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 120,
   preferred_humidity = 100,
   preferred_fertility = 200,
   pickiness = 50,
}

descriptions:new_immovable_type {
   name = "berry_bush_currant_black_tiny",
   descname = _ "Black Currant (tiny)",
   size = "small",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "grow=berry_bush_currant_black_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {4, 10},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

descriptions:new_immovable_type {
   name = "berry_bush_currant_black_small",
   descname = _ "Black Currant (small)",
   size = "small",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:24s",
         "grow=berry_bush_currant_black_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {8, 20},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

descriptions:new_immovable_type {
   name = "berry_bush_currant_black_medium",
   descname = _ "Black Currant (medium)",
   size = "small",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:22s",
         "grow=berry_bush_currant_black_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {13, 33},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

descriptions:new_immovable_type {
   name = "berry_bush_currant_black_ripe",
   descname = _ "Black Currant (ripe)",
   size = "small",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:3m20s",
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
         hotspot = {13, 33},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
