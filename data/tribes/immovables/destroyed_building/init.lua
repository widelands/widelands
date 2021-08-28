push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "destroyed_building",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Destroyed building"),
   icon = dirname .. "menu.png",
   size = "big",
   programs = {
      main = {
         "animate=idle duration:30s",
         "transform=ashes",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         fps = 10,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 40, 57 }
      },
   }
}

pop_textdomain()
