dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 120,
   preferred_humidity = 100,
   preferred_fertility = 200,
   pickiness = 50,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_black_tiny",
   descname = _ "Black Currant (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "tiny/menu.png",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 20000",
         "remove=10",
         "grow=berry_bush_currant_black_small",
      },
   },
   animations = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {4, 10}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_black_small",
   descname = _ "Black Currant (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "small/menu.png",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 24000",
         "remove=30",
         "grow=berry_bush_currant_black_medium",
      },
   },
   animations = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {8, 20}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_black_medium",
   descname = _ "Black Currant (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "medium/menu.png",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 22000",
         "remove=70",
         "grow=berry_bush_currant_black_ripe",
      },
   },
   animations = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {13, 33}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_black_ripe",
   descname = _ "Black Currant (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "ripe/menu.png",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 200000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = {
      idle = {
         directory = dirname .. "ripe",
         basename = "idle",
         hotspot = {13, 33}
      }
   }
}
