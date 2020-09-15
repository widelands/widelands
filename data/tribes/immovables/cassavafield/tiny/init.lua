push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "cassavarootfield_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cassavaroot Field (tiny)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:30s",
         "transform=cassavarootfield_small",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {3, 7},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 2
      }
   }
}

pop_textdomain()
