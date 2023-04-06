push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "barleyfield_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (tiny)"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:2m30s",
         "transform=barleyfield_small",
      }
   },
   animations = {
      idle = {
         hotspot = {21, 13}
      }
   }
}

pop_textdomain()
