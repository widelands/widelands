push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "blackrootfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Blackroot Field (harvested)"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   programs = {
      main = {
         "animate=idle duration:50s",
         "remove=",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 26, 16 },
      },
   }
}

pop_textdomain()
