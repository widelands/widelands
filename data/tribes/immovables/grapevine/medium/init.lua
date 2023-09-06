push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
   name = "grapevine_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Grapevine (medium)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:40s",
         "transform=grapevine_ripe",
      }
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 17, 27 },
      },
   }
}

pop_textdomain()
