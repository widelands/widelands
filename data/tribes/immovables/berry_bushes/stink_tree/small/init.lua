push_textdomain("tribes")

local dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_stink_tree_small",
   descname = _("Stink Shepherd’s Tree (small)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:50s",
         "grow=berry_bush_stink_tree_medium",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {26, 22},
         frames = 10,
         columns = 5,
         rows = 2,
      }
   }
}

pop_textdomain()
