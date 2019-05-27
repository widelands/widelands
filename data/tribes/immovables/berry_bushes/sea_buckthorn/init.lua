dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 100, -- Temperature is in arbitrary units.
   preferred_humidity = 950,    -- Values between 0 and 1000 (1000 being very wet).
   preferred_fertility = 50,    -- Values between 0 and 1000 (1000 being very fertile).
   pickiness = 15,              -- Lower means it is less picky, i.e. it can deal better.
}

animations = {}
add_animation(animations, "idle", dirname, "tiny/idle", { 9, 19 })

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
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname, "small/idle", { 14, 32 })

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
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname, "medium/idle", { 17, 40 })

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
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname, "ripe/idle", { 17, 40 })

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
   animations = animations,
}
