push_textdomain("tribes")

dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 125,
   preferred_humidity = 220,
   preferred_fertility = 800,
   pickiness = 35,
}

descriptions:new_immovable_type {
   name = "berry_bush_stink_tree_tiny",
   descname = _ "Stink Shepherd’s Tree (tiny)",
   size = "small",
   icon = dirname .. "tiny/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:30s",
         "grow=berry_bush_stink_tree_small",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "tiny",
         basename = "idle",
         hotspot = {12, 10},
         frames = 10,
         columns = 5,
         rows = 2,
      }
   }
}

descriptions:new_immovable_type {
   name = "berry_bush_stink_tree_small",
   descname = _ "Stink Shepherd’s Tree (small)",
   size = "small",
   icon = dirname .. "small/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:50s",
         "grow=berry_bush_stink_tree_medium",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "small",
         basename = "idle",
         hotspot = {26, 22},
         frames = 10,
         columns = 5,
         rows = 2,
      }
   }
}

descriptions:new_immovable_type {
   name = "berry_bush_stink_tree_medium",
   descname = _ "Stink Shepherd’s Tree (medium)",
   size = "small",
   icon = dirname .. "medium/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m10s",
         "grow=berry_bush_stink_tree_ripe",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname .. "medium",
         basename = "idle",
         hotspot = {36, 34},
         frames = 10,
         columns = 5,
         rows = 2,
      }
   }
}

descriptions:new_immovable_type {
   name = "berry_bush_stink_tree_ripe",
   descname = _ "Stink Shepherd’s Tree (ripe)",
   size = "small",
   icon = dirname .. "ripe/menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:8m20s",
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
         hotspot = {36, 34},
         frames = 10,
         columns = 5,
         rows = 2,
      }
   }
}

pop_textdomain()
