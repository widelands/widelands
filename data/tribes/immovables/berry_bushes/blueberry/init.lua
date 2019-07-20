set_textdomain("tribes")

dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 45, -- Temperature is in arbitrary units.
   preferred_humidity = 750,   -- Values between 0 and 1000 (1000 being very wet).
   preferred_fertility = 400,  -- Values between 0 and 1000 (1000 being very fertile).
   pickiness = 15,             -- Lower means it is less picky, i.e. it can deal better.
}

animations = {}
add_animation(animations, "idle", dirname .. "tiny", "idle", { 6, 7 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_tiny",
   descname = _ "Blueberry Bush (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "tiny/menu.png",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 18000",
         "remove=50",
         "grow=berry_bush_blueberry_small",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "small", "idle", { 12, 13 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_small",
   descname = _ "Blueberry Bush (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "small/menu.png",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 36000",
         "remove=50",
         "grow=berry_bush_blueberry_medium",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "medium", "idle", { 15, 16 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_medium",
   descname = _ "Blueberry Bush (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "medium/menu.png",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 24000",
         "remove=50",
         "grow=berry_bush_blueberry_ripe",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "ripe", "idle", { 15, 16 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_ripe",
   descname = _ "Blueberry Bush (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "ripe/menu.png",
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
   animations = animations,
}
