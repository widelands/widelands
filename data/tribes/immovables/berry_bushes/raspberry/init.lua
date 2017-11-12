dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 85,
   preferred_humidity = 0.65,
   preferred_fertility = 0.75,
   pickiness = 0.65,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_tiny",
   descname = _ "Raspberry Bush (Tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 22000",
         "transform=berry_bush_raspberry_small",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "tiny/idle_??.png"),
         hotspot = { 7, 16 },
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_small",
   descname = _ "Raspberry Bush (Small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 28000",
         "transform=berry_bush_raspberry_medium",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "small/idle_??.png"),
         hotspot = { 15, 33 }
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_medium",
   descname = _ "Raspberry Bush (Medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 40000",
         "transform=berry_bush_raspberry_ripe",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "medium/idle_??.png"),
         hotspot = { 15, 33 },
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_ripe",
   descname = _ "Raspberry Bush (Ripe)",
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
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "ripe/idle_??.png"),
         hotspot = { 15, 33 },
      },
   },
}
