push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

descriptions:new_immovable_type {
   name = "frisians_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Gold"),
   icon = dirname .. "pics/gold_few.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "gold_few",
         hotspot = {0, 43},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

pop_textdomain()
