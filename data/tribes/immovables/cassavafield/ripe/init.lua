push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "cassavafield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cassava Field (ripe)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      },
      harvest = {
         "transform=cassavafield_harvested",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {15, 36},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 2
      }
   }
}

pop_textdomain()
