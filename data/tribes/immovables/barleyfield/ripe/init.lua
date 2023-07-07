push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "barleyfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (ripe)"),
   animation_directory = dirname,
   size = "small",
   icon = dirname .. "menu.png",
   programs = {
      main = {
         "animate=idle duration:20m50s",
         "remove=",
      },
      harvest = {
         "transform=barleyfield_harvested"
      }
   },
   animations = {
      idle = {
         hotspot = {21, 34}
      }
   }
}

pop_textdomain()
