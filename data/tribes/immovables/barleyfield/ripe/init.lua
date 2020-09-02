push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "barleyfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (ripe)"),
   size = "small",
   icon = dirname .. "menu.png",
   helptext_script = dirname .. "helptexts.lua",
   programs = {
      main = {
         "animate=idle duration:41m40s",
         "remove=",
      },
      harvest = {
         "transform=barleyfield_harvested"
      }
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {21, 34}
      }
   }
}

pop_textdomain()
