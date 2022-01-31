push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type {
   name = "berry_bush_strawberry_tiny",
   descname = _("Strawberries (tiny)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:16s",
         "grow=berry_bush_strawberry_small",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {4, 4},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
