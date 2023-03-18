push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "birch_summer_old",
   descname = _("Birch (Old)"),
   species = _("Birch"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:13m20s",
         "transform=deadtree2 chance:10.55%",
         "seed=birch_summer_sapling proximity:23.44%",
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
         hotspot = { 23, 58 },
         sound_effect = {
            path = "sound/animals/bird5",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
