push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type {
   name = "berry_bush_stink_tree_medium",
   descname = _("Stink Shepherd’s Tree (medium)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m10s",
         "grow=berry_bush_stink_tree_ripe",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {36, 34},
         frames = 10,
         columns = 5,
         rows = 2,
      }
   }
}

pop_textdomain()
