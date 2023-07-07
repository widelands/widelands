push_textdomain("world")

local dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "mushroom_dark_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _("Dark Mushroom Tree (Old)"),
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _("Dark Mushroom Tree"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:17m30s",
         "transform=deadtree2 chance:9.76%",
         "seed=mushroom_dark_wasteland_sapling proximity:78.12%",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         basename = "old",
         fps = 15,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 24, 60 }
      }
   },
}

pop_textdomain()
