push_textdomain("world")

local dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "palm_borassus_desert_old",
   descname = _("Borassus Palm (Old)"),
   species = _("Borassus Palm"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:33m20s",
         "transform=deadtree5 chance:9.76%",
         "seed=palm_borassus_desert_sapling proximity:62.5%",
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
         hotspot = { 24, 60 },
         sound_effect = {
            path = "sound/animals/crickets1",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
