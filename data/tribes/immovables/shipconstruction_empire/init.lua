push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "empire_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "construct=idle duration:5s decay_after:3m30s",
         "transform=bob:empire_ship",
      }
   },
   buildcost = {
      planks = 10,
      log = 2,
      cloth = 4
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         fps = 1,
         frames = 7,
         rows = 4,
         columns = 2,
         representative_frame = 5,
         hotspot = { 115, 78 }
      },
   }
}

pop_textdomain()
