push_textdomain("tribes")

local dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_currant_black_tiny",
   descname = _("Black Currant (tiny)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "grow=berry_bush_currant_black_small",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {4, 10},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
