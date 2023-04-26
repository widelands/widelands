push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

wl.Descriptions():new_immovable_type {
   name = "barbarians_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Iron"),
   icon = dirname .. "pics/iron_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "iron_few",
         hotspot = {8, 27}
      }
   }
}

pop_textdomain()
