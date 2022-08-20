push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "larch_summer_old",
   descname = _("Larch (Old)"),
   species = _("Larch"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:24m15s",
         "transform=deadtree3 chance:8.98%",
         "seed=larch_summer_sapling proximity:11.72%",
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
         hotspot = { 15, 59 },
         sound_effect = {
            path = "sound/animals/bird6",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
