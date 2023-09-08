push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "cornfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (harvested)"),
   icon = dirname .. "menu.png",
   programs = {
      main = {
         "animate=idle duration:50s",
         "remove=",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 5,
         rows = 1,
         columns = 5,
         hotspot = { 36, 41 }
      },
   },
}

pop_textdomain()
