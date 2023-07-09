push_textdomain("tribes")

local dirname = path.dirname(__file__) .. "../"

wl.Descriptions():new_immovable_type {
   name = "barbarians_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Coal"),
   icon = dirname .. "pics/coal_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "coal_much",
         hotspot = {8, 27}
      }
   }
}

pop_textdomain()
