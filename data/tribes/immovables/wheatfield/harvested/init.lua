push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "wheatfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Wheat Field (harvested)"),
   icon = dirname .. "menu.png",
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 33, 22 },
      },
   }
}

pop_textdomain()
