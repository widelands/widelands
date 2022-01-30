push_textdomain("tribes")

dirname = path.dirname (__file__)

include(dirname .. "../terrain_affinity.lua")

descriptions:new_immovable_type {
   name = "berry_bush_strawberry_medium",
   descname = _("Strawberries (medium)"),
   size = "small",
   icon = dirname .. "menu.png",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:50s",
         "grow=berry_bush_strawberry_ripe",
      },
   },
   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {21, 15},
         frames = 4,
         columns = 2,
         rows = 2,
      }
   }
}

pop_textdomain()
