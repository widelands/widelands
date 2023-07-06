push_textdomain("tribes")

local dirname = path.dirname(__file__) .. "../"

wl.Descriptions():new_immovable_type {
   name = "amazons_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   icon = dirname .. "pics/none_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },

   animation_directory = dirname .. "pics",
   animations = { idle = { basename = "none", hotspot = {7, 32}}}
}

pop_textdomain()
