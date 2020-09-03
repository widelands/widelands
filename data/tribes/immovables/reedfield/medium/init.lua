push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "reedfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Reed Field (medium)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:40s",
         "transform=reedfield_ripe",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 22, 21 },
      },
   }
}

pop_textdomain()
