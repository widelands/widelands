push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_stink_tree_tiny",
   descname = _("Stink Shepherd’s Tree (tiny)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:30s",
         "grow=berry_bush_stink_tree_small",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {12, 10},
         frames = 10,
         columns = 5,
         rows = 2,
      }
   }
}

pop_textdomain()
