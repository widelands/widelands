push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

wl.Descriptions():new_immovable_type {
   name = "empire_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Marble"),
   icon = dirname .. "pics/stone_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "stone_much",
         hotspot = {9, 19}
      }
   }
}

pop_textdomain()
