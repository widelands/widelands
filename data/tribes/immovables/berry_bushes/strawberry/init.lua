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
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "tiny/menu.png",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 16000",
         "remove=25",
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
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "small/menu.png",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 22000",
         "remove=10",
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
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "medium/menu.png",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 50000",
         "remove=15",
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
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "ripe/menu.png",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 450000",
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
