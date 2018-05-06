dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 115,
   preferred_humidity = 0.6,
   preferred_fertility = 0.2,
   pickiness = 0.15,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_tiny",
   descname = _ "Juniper (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 10000",
         "remove=60",
         "grow=berry_bush_juniper_small",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "tiny/idle_?.png"),
         hotspot = {3, 15},
         scale = 2,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_small",
   descname = _ "Juniper (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 10000",
         "remove=20",
         "grow=berry_bush_juniper_medium",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "small/idle_?.png"),
         hotspot = {6, 30},
         scale = 2,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_medium",
   descname = _ "Juniper (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 70000",
         "remove=10",
         "grow=berry_bush_juniper_ripe",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "medium/idle_?.png"),
         hotspot = {10, 47},
         scale = 2,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_juniper_ripe",
   descname = _ "Juniper (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 800000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "ripe/idle_?.png"),
         hotspot = {11, 49},
         scale = 2,
      },
   },
}
