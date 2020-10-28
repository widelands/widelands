push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "cassavarootfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cassava Root Field (medium)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:50s",
         "transform=cassavarootfield_ripe",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {11, 25},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 2
      }
   }
}

pop_textdomain()
