push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "liana_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _("Liana Tree (Old)"),
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _("Liana Tree"),
   icon = dirname .. "../menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:24m15s",
         "transform=deadtree4 chance:18.75%",
         "seed=liana_wasteland_sapling proximity:39%",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 10,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 24, 60 }
      }
   },
}

pop_textdomain()
