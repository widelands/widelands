push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

descriptions:new_immovable_type {
   name = "amazons_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Granite"),
   icon = dirname .. "pics/stones_2_2.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },

   animation_directory = dirname .. "pics",
   animations = { idle = { basename = "stones_2", hotspot = {7, 32}}}
}

pop_textdomain()
