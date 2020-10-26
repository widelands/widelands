push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "cornfield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (small)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:45s",
         "transform=cornfield_medium",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 34, 24 },
      },
   }
}

pop_textdomain()
