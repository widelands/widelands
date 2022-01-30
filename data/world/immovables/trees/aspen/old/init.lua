push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "aspen_summer_old",
   descname = _("Aspen (Old)"),
   species = _("Aspen"),
   icon = dirname .. "../menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:17m30s",
         "transform=deadtree2 chance:5.86%",
         "seed=aspen_summer_sapling proximity:39%",
      },
      fall = {
         "animate=falling duration:1s400ms",
         "transform=fallentree",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 10,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 23, 58 }
      },
      falling = {
         directory = dirname,
         basename = "falling",
         fps = 10,
         frames = 9,
         rows = 3,
         columns = 3,
         hotspot = { 20, 59 },
         play_once = true
      }
   },
}

pop_textdomain()
