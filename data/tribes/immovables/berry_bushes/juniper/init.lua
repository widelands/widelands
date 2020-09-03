push_textdomain("tribes")

dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 115,
   preferred_humidity = 600,
   preferred_fertility = 200,
   pickiness = 15,
}

tribes:new_immovable_type {
   name = "berry_bush_juniper_tiny",
   descname = _ "Juniper (tiny)",
   size = "small",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:10s",
         "grow=berry_bush_juniper_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {3, 15},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_juniper_small",
   descname = _ "Juniper (small)",
   size = "small",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:10s",
         "grow=berry_bush_juniper_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {6, 30},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_juniper_medium",
   descname = _ "Juniper (medium)",
   size = "small",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m10s",
         "grow=berry_bush_juniper_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {10, 47},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   name = "berry_bush_juniper_ripe",
   descname = _ "Juniper (ripe)",
   size = "small",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:13m20s",
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
         hotspot = {11, 49},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
