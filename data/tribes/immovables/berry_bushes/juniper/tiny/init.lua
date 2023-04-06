push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_juniper_tiny",
   descname = _("Juniper (tiny)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:10s",
         "grow=berry_bush_juniper_small",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {3, 15},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
