dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 110, -- Temperature is in arbitrary units.
   preferred_humidity = 0.6,    -- In percent (1 being very wet).
   preferred_fertility = 0.8,   -- In percent (1 being very fertile).
   pickiness = 0.2,             -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_tiny",
   descname = _ "Strawberries (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 16000",
         "remove=25",
         "grow=berry_bush_strawberry_small",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "tiny/idle_?.png"),
         hotspot = {4, 4},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_small",
   descname = _ "Strawberries (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 22000",
         "remove=10",
         "grow=berry_bush_strawberry_medium",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "small/idle_?.png"),
         hotspot = {12, 9},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_medium",
   descname = _ "Strawberries (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 50000",
         "remove=15",
         "grow=berry_bush_strawberry_ripe",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "medium/idle_?.png"),
         hotspot = {21, 15},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_ripe",
   descname = _ "Strawberries (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
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
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "ripe/idle_?.png"),
         hotspot = {21, 15},
         scale = 3,
      },
   },
}
