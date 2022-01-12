push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type {
   name = "berry_bush_raspberry_small",
   descname = _ "Raspberry Bush (small)",
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "grow=berry_bush_raspberry_medium",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {16, 31},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
