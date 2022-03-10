push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_blueberry_tiny",
   descname = _("Blueberry Bush (tiny)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:18s",
         "grow=berry_bush_blueberry_small",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {6, 7},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
