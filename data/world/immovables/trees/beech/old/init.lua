push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "beech_summer_old",
   descname = _("Beech (Old)"),
   species = _("Beech"),
   icon = dirname .. "../menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m25s",
         "transform=deadtree2 chance:7.81%",
         "seed=beech_summer_sapling proximity:97.66%",
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
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 24, 60 },
         sound_effect = {
            path = "sound/animals/bird6",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
