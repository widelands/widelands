dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 70, -- Temperature is in arbitrary units.
   preferred_humidity = 0.75,  -- In percent (1 being very wet).
   preferred_fertility = 0.1,  -- In percent (1 being very fertile).
   pickiness = 0.5,            -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_tiny",
   descname = _ "Blueberry Bush (Tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 18000",
         "remove=50",
         "grow=berry_bush_blueberry_small",
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
   name = "berry_bush_blueberry_small",
   descname = _ "Blueberry Bush (Small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 36000",
         "remove=50",
         "grow=berry_bush_blueberry_medium",
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
   name = "berry_bush_blueberry_medium",
   descname = _ "Blueberry Bush (Medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 24000",
         "remove=50",
         "grow=berry_bush_blueberry_ripe",
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
   name = "berry_bush_blueberry_ripe",
   descname = _ "Blueberry Bush (Ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 600000",
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
