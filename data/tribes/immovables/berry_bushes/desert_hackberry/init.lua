dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 170, -- Temperature is in arbitrary units.
   preferred_humidity = 50,     -- Values between 0 and 1000 (1000 being very wet).
   preferred_fertility = 50,    -- Values between 0 and 1000 (1000 being very fertile).
   pickiness = 30,             -- Lower means it is less picky, i.e. it can deal better.
}

animations = {}
add_animation(animations, "idle", dirname, "tiny/idle", { 10, 9 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_desert_hackberry_tiny",
   descname = _ "Desert Hackberry (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 80000",
         "remove=10",
         "grow=berry_bush_desert_hackberry_small",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname, "small/idle", { 19, 21 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_desert_hackberry_small",
   descname = _ "Desert Hackberry (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=15",
         "grow=berry_bush_desert_hackberry_medium",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname, "medium/idle", { 21, 27 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_desert_hackberry_medium",
   descname = _ "Desert Hackberry (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=20",
         "grow=berry_bush_desert_hackberry_ripe",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname, "ripe/idle", { 21, 27 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_desert_hackberry_ripe",
   descname = _ "Desert Hackberry (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 500000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = animations,
}
