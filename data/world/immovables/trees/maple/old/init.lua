push_textdomain("world")

local dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "maple_winter_old",
   descname = _("Maple (Old)"),
   species = _("Maple"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree4 chance:15.23%",
         "seed=maple_winter_sapling proximity:93.75%",
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
            path = "sound/animals/bird4",
            priority = "10%"
         },
      },
   },
}

pop_textdomain()
