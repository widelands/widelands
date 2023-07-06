push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
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
         frames = 10,
         columns = 10,
         rows = 1,
         hotspot = { 43, 60 }
      },
   }
}

pop_textdomain()
