dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 50,
   preferred_humidity = 300,
   preferred_fertility = 50,
   pickiness = 10,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_tiny",
   descname = _ "Raspberry Bush (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "grow=berry_bush_raspberry_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {4, 7},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_small",
   descname = _ "Raspberry Bush (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "grow=berry_bush_raspberry_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {16, 31},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_medium",
   descname = _ "Raspberry Bush (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:40s",
         "grow=berry_bush_raspberry_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {17, 34},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_ripe",
   descname = _ "Raspberry Bush (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:6m40s",
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
         hotspot = {17, 34},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}
