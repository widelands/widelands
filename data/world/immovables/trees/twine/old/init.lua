push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "twine_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _("Twine Tree (Old)"),
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _("Twine Tree"),
   icon = dirname .. "../menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree3 chance:14.06%",
         "seed=twine_wasteland_sapling proximity:7.8%",
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
