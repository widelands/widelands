push_textdomain("tribes")

local dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_stink_tree_ripe",
   descname = _("Stink Shepherd’s Tree (ripe)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      },
      harvest = {
         "remove=",
      }
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
