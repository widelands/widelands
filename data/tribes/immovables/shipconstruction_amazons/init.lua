push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_immovable_type {
   name = "amazons_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "construct=idle duration:5s decay_after:3m30s",
         "transform=bob:amazons_ship",
      }
   },
   buildcost = {
      log = 5,
      balsa = 5,
      rubber = 3,
      rope = 3
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {73, 37},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

pop_textdomain()
