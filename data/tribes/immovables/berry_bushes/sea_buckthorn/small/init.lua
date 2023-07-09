push_textdomain("tribes")

local dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_sea_buckthorn_small",
   descname = _("Sea Buckthorn (small)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:18s",
         "grow=berry_bush_sea_buckthorn_medium",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {14, 32},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
