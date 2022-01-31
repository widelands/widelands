push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "aspen_summer_mature",
   descname = _("Aspen (Mature)"),
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:11.72%",
         "seed=aspen_summer_sapling proximity:23.44%",
         "animate=idle duration:20s",
         "remove=chance:7.81%",
         "grow=aspen_summer_old",
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
         hotspot = { 18, 47 }
      }
   },
}

pop_textdomain()
