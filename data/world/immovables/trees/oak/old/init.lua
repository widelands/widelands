push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "oak_summer_old",
   descname = _("Oak (Old)"),
   species = _("Oak"),
   icon = dirname .. "../menu.png",
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:37m30s",
         "transform=deadtree2 chance:4.69%",
         "seed=oak_summer_sapling proximity:39%",
      },
      fall = {
         "animate=falling duration:1s400ms",
         "transform=fallentree",
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
            path = "sound/animals/bird2",
            priority = "10%"
         },
      },
      falling = {
         fps = 10,
         frames = 7,
         rows = 4,
         columns = 2,
         hotspot = { 10, 60 },
         play_once = true
      }
   },
}

pop_textdomain()
