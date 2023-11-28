push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type {
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

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 5,
         rows = 1,
         columns = 5,
         hotspot = { 25, 30 }
      },
   }
}

pop_textdomain()
