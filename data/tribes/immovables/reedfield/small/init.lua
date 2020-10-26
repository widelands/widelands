push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "reedfield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Reed Field (small)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:28s",
         "transform=reedfield_medium",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 23, 20 },
      },
   }
}

pop_textdomain()
