push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ship_type {
   name = "barbarians_ship",
   -- TRANSLATORS: This is the Barbarians' ship's name used in lists of units
   descname = pgettext("barbarians_ship", "Ship"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,

   animations = {
      idle = {
         hotspot = { 115, 76 },
         fps = 10
      },
      sail = {
         hotspot = { 115, 76 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
