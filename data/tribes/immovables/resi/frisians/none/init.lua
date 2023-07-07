push_textdomain("tribes")

local dirname = path.dirname(__file__) .. "../"

wl.Descriptions():new_immovable_type {
   name = "frisians_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   icon = dirname .. "pics/none.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "none",
         hotspot = {0, 43},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

pop_textdomain()
