push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "palm_date_desert_old",
   descname = _("Date Palm (Old)"),
   species = _("Date Palm"),
   icon = dirname .. "../menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:17m30s",
         "transform=deadtree5 chance:12.5%",
         "seed=palm_date_desert_sapling proximity:78.12%",
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
            path = "sound/animals/crickets1",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()
