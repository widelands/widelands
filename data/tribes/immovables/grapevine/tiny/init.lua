push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "grapevine_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Grapevine (tiny)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:22s",
         "transform=grapevine_small",
      }
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 15, 18 },
      },
   }
}

pop_textdomain()
