push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ship_type {
   name = "amazons_ship",
   -- TRANSLATORS: This is the ship's name used in lists of units
   descname = _"Ship",
   capacity = 30,
   vision_range = 4,
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {73, 49},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      sinking = {
         hotspot = {73, 49},
         fps = 7,
         frames = 21,
         columns = 3,
         rows = 7
      },
      sail = {
         directional = true,
         hotspot = {80, 52},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}

pop_textdomain()

