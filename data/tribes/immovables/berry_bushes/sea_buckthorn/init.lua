dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 100, -- Temperature is in arbitrary units.
   preferred_humidity = 0.95,   -- In percent (1 being very wet).
   preferred_fertility = 0.05,  -- In percent (1 being very fertile).
   pickiness = 0.15,            -- Lower means it is less picky, i.e. it can deal better.
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_tiny",
   descname = _ "Sea Buckthorn (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 14000",
         "remove=15",
         "grow=berry_bush_sea_buckthorn_small",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "tiny/idle_?.png"),
         hotspot = {9, 19},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_small",
   descname = _ "Sea Buckthorn (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 18000",
         "remove=10",
         "grow=berry_bush_sea_buckthorn_medium",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "small/idle_?.png"),
         hotspot = {14, 32},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_medium",
   descname = _ "Sea Buckthorn (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 26000",
         "remove=5",
         "grow=berry_bush_sea_buckthorn_ripe",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "medium/idle_?.png"),
         hotspot = {17, 40},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_sea_buckthorn_ripe",
   descname = _ "Sea Buckthorn (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 300000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "ripe/idle_?.png"),
         hotspot = {17, 40},
         scale = 3,
      },
   },
}
