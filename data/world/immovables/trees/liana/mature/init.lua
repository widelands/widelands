push_textdomain("world")

local dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "liana_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _("Liana Tree (Mature)"),
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:3.91%",
         "seed=liana_wasteland_sapling proximity:11.72%",
         "animate=idle duration:30s",
         "remove=chance:3.91%",
         "grow=liana_wasteland_old",
      },
   },
   spritesheets = {
      idle = {
         basename = "mature",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 18, 48 }
      }
   },
}

pop_textdomain()
