dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 100, -- Temperature is in arbitrary units.
   preferred_humidity = 950,    -- Values between 0 and 1000 (1000 being very wet).
   preferred_fertility = 50,    -- Values between 0 and 1000 (1000 being very fertile).
   pickiness = 15,              -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_tiny",
   descname = _ "Sea Buckthorn (tiny)",
   size = "small",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:14s",
         "grow=berry_bush_sea_buckthorn_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {9, 19},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_small",
   descname = _ "Sea Buckthorn (small)",
   size = "small",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:18s",
         "grow=berry_bush_sea_buckthorn_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {14, 32},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_medium",
   descname = _ "Sea Buckthorn (medium)",
   size = "small",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:26s",
         "grow=berry_bush_sea_buckthorn_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {17, 40},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_ripe",
   descname = _ "Sea Buckthorn (ripe)",
   size = "small",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:5m",
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
         hotspot = {17, 40},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}
