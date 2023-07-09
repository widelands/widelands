push_textdomain("world")

local dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "palm_coconut_desert_old",
   descname = _("Coconut Palm (Old)"),
   species = _("Coconut Palm"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree6 chance:14.06%",
         "seed=palm_coconut_desert_sapling proximity:39%",
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
         hotspot = { 24, 59 },
         sound_effect = {
            path = "sound/animals/bird3",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
