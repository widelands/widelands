push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_sea_buckthorn_medium",
   descname = _("Sea Buckthorn (medium)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:26s",
         "grow=berry_bush_sea_buckthorn_ripe",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {17, 40},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
