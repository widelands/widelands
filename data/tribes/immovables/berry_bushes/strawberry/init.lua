dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 110, -- Temperature is in arbitrary units.
   preferred_humidity = 600,    -- Values between 0 and 1000 (1000 being very wet).
   preferred_fertility = 800,   -- Values between 0 and 1000 (1000 being very fertile).
   pickiness = 20,             -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_tiny",
   descname = _ "Strawberries (tiny)",
   size = "small",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:16s",
         "grow=berry_bush_strawberry_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {4, 4},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_small",
   descname = _ "Strawberries (small)",
   size = "small",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:22s",
         "grow=berry_bush_strawberry_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {12, 9},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_medium",
   descname = _ "Strawberries (medium)",
   size = "small",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:50s",
         "grow=berry_bush_strawberry_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {21, 15},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_ripe",
   descname = _ "Strawberries (ripe)",
   size = "small",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:7m30s",
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
         hotspot = {21, 15},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}
