push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "spruce_summer_old",
   descname = _("Spruce (Old)"),
   species = _("Spruce"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree3 chance:9.37%",
         "seed=spruce_summer_sapling proximity:78.12%",
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
            path = "sound/animals/bird3",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
