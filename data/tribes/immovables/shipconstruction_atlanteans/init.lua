push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "atlanteans_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "construct=idle duration:5s decay_after:3m30s",
         "transform=bob:atlanteans_ship",
      }
   },
   buildcost = {
      planks = 10,
      log = 2,
      spidercloth = 4
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         fps = 1,
         frames = 7,
         rows = 4,
         columns = 2,
         representative_frame = 5,
         hotspot = { 79, 73 }
      },
   }
}

pop_textdomain()
