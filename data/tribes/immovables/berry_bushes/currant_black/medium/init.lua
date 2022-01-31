push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type {
   name = "berry_bush_currant_black_medium",
   descname = _("Black Currant (medium)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:22s",
         "grow=berry_bush_currant_black_ripe",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {13, 33},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
