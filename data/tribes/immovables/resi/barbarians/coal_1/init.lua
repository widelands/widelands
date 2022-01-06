push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

descriptions:new_immovable_type {
   name = "barbarians_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Coal"),
   icon = dirname .. "pics/coal_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "coal_few",
         hotspot = {8, 27}
      }
   }
}

pop_textdomain()
