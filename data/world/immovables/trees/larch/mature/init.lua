push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "larch_summer_mature",
   descname = _ "Larch (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:9.37%",
         "grow=larch_summer_old",
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
         hotspot = { 12, 48 }
      }
   },
}

pop_textdomain()
