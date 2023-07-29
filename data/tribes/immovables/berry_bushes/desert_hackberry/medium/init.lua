push_textdomain("tribes")

local dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type {
   name = "berry_bush_desert_hackberry_medium",
   descname = _("Desert Hackberry (medium)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "grow=berry_bush_desert_hackberry_ripe",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {21, 27},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
