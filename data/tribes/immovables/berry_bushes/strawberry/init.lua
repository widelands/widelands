dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 120, -- Temperature is in arbitrary units.
   preferred_humidity = 0.9,    -- In percent (1 being very wet).
   preferred_fertility = 0.65,  -- In percent (1 being very fertile).
   pickiness = 0.4,             -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_tiny",
   descname = _ "Strawberry Bush (Tiny)",
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
         hotspot = {21, 17},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_small",
   descname = _ "Strawberry Bush (Small)",
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
         hotspot = {21, 17},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_medium",
   descname = _ "Strawberry Bush (Medium)",
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
         hotspot = {21, 17},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_strawberry_ripe",
   descname = _ "Strawberry Bush (Ripe)",
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
         hotspot = {21, 17},
         scale = 3,
      },
   },
}
