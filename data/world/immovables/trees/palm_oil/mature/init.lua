push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "palm_oil_desert_mature",
   descname = _("Oil Palm (Mature)"),
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25s",
         "remove=chance:3.91%",
         "seed=palm_oil_desert_sapling proximity:31.25%",
         "animate=idle duration:30s",
         "remove=chance:3.91%",
         "grow=palm_oil_desert_old",

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
