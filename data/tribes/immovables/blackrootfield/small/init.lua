push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "blackrootfield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Blackroot Field (small)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:45s",
         "transform=blackrootfield_medium",
      }
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 26, 16 },
      },
   }
}

pop_textdomain()
