push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_currant_red_small",
   descname = _("Red Currant (small)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:28s",
         "grow=berry_bush_currant_red_medium",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {8, 20},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
