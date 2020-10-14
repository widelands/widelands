push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "grapevine_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Grapevine (medium)"),
   icon = dirname .. "menu.png",
   size = "medium",
   programs = {
      main = {
         "animate=idle duration:40s",
         "transform=grapevine_ripe",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 15, 18 },
      },
   }
}

pop_textdomain()
