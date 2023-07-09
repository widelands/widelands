push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "barleyfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (harvested)"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   programs = {
      main = {
         "animate=idle duration:16m40s",
         "remove=",
      }
   },
   animations = {
      idle = {
         hotspot = {21, 34}
      }
   }
}

pop_textdomain()
