push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "cassavafield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cassava Field (small)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:45s",
         "transform=cassavafield_medium",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {6, 14},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 2
      }
   }
}

pop_textdomain()
