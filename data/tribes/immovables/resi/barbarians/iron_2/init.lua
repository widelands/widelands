push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

descriptions:new_immovable_type {
   name = "barbarians_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Iron"),
   icon = dirname .. "pics/iron_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "iron_much",
         hotspot = {8, 27}
      }
   }
}

pop_textdomain()
