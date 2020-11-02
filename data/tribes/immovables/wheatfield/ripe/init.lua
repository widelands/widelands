push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "wheatfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Wheat Field (ripe)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      },
      harvest = {
         "transform=wheatfield_harvested"
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 31, 24 },
      },
   }
}

pop_textdomain()
