push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

wl.Descriptions():new_immovable_type {
   name = "atlanteans_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Crystals"),
   icon = dirname .. "pics/stone_few.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "stone_few",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

pop_textdomain()
