dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 95,
   preferred_humidity = 550,
   preferred_fertility = 450,
   pickiness = 40,
}

animations = {}
add_animation(animations, "idle", dirname .. "tiny", "idle", { 4, 10 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_red_tiny",
   descname = _ "Red Currant (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "tiny/menu.png",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 24000",
         "remove=30",
         "grow=berry_bush_currant_red_small",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "small", "idle", { 8, 20 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_red_small",
   descname = _ "Red Currant (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "small/menu.png",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 28000",
         "remove=70",
         "grow=berry_bush_currant_red_medium",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "medium", "idle", { 13, 33 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_red_medium",
   descname = _ "Red Currant (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "medium/menu.png",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 26000",
         "remove=90",
         "grow=berry_bush_currant_red_ripe",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "ripe", "idle", { 13, 33 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_red_ripe",
   descname = _ "Red Currant (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "ripe/menu.png",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 250000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = animations,
}
