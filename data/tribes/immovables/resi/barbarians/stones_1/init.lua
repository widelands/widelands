push_textdomain("tribes")

local dirname = path.dirname(__file__) .. "../"

wl.Descriptions():new_immovable_type {
   name = "barbarians_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Granite"),
   icon = dirname .. "pics/stone_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "stone_few",
         hotspot = {8, 27}
      }
   }
}

pop_textdomain()
