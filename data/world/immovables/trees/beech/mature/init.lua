push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "beech_summer_mature",
   descname = _ "Beech (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=beech_summer_old",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "mature",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 18, 48 }
      }
   },
}

pop_textdomain()
