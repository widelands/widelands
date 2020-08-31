push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
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

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 115, 82 },
         fps = 1,
         representative_frame = 5
      },
   }
}

pop_textdomain()
