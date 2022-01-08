push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type {
   name = "berry_bush_desert_hackberry_small",
   descname = _ "Desert Hackberry (small)",
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "grow=berry_bush_desert_hackberry_medium",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {19, 21},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
