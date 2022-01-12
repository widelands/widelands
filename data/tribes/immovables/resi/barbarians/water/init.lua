push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

descriptions:new_immovable_type {
   name = "barbarians_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Water"),
   icon = dirname .. "pics/water_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "water",
         hotspot = {8, 27}
      }
   }
}

pop_textdomain()
