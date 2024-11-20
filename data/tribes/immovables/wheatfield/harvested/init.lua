push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "wheatfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Wheat Field (harvested)"),
   icon = dirname .. "menu.png",
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 5,
         rows = 1,
         columns = 5,
         hotspot = { 36, 27 }
      },
   }
}

pop_textdomain()
