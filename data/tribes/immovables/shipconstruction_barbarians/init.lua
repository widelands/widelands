push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "barbarians_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "construct=idle duration:5s decay_after:3m30s",
         "transform=bob:barbarians_ship",
      }
   },
   buildcost = {
      blackwood = 10,
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
         hotspot = { 94, 75 }
      },
   }
}

pop_textdomain()
