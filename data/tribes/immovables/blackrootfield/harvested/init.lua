push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "blackrootfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Blackroot Field (harvested)"),
   icon = dirname .. "menu.png",
   programs = {
      main = {
         "animate=idle duration:50s",
         "remove=",
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
