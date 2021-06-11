push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "destroyed_building",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Destroyed building"),
   icon = dirname .. "menu.png",
   size = "big",
   programs = {
      main = {
         "animate=idle duration:30s",
         "transform=ashes",
      }
   },

   animation_directory = dirname,
   animations = {
      idle = {
         basename = "burn",
         hotspot = { 40, 57 },
         fps = 10,
      },
   }
}

pop_textdomain()
