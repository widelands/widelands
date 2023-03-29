push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "rowan_summer_old",
   descname = _("Rowan (Old)"),
   species = _("Rowan"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:23m52s",
         "transform=deadtree4 chance:10.16%",
         "seed=rowan_summer_sapling proximity:70.31%",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         basename = "old",
         fps = 10,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 23, 59 },
         sound_effect = {
            path = "sound/animals/bird6",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
