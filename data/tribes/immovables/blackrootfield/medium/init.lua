push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "blackrootfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Blackroot Field (medium)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:50s",
         "transform=blackrootfield_ripe",
      }
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 26, 30 },
      },
   }
}

pop_textdomain()
