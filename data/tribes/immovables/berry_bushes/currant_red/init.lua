dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 95,
   preferred_humidity = 550,
   preferred_fertility = 450,
   pickiness = 40,
}

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
      main = {
         "animate=idle duration:24s",
         "grow=berry_bush_currant_red_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {4, 10},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

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
      main = {
         "animate=idle duration:28s",
         "grow=berry_bush_currant_red_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {8, 20},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

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
      main = {
         "animate=idle duration:26s",
         "grow=berry_bush_currant_red_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {13, 33},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

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
      main = {
         "animate=idle duration:4m10s",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "ripe",
         basename = "idle",
         hotspot = {13, 33},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}
