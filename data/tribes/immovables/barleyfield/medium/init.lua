push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "barleyfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (medium)"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:4m10s",
         "transform=barleyfield_ripe",
      }
   },
   animations = {
      idle = {
         hotspot = {21, 33}
      }
   }
}

pop_textdomain()
