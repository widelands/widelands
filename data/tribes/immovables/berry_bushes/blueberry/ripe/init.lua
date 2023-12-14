push_textdomain("tribes")

local dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_blueberry_ripe",
   descname = _("Blueberry Bush (ripe)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {15, 16},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}
pop_textdomain()
