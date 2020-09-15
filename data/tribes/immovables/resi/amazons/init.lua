push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "amazons_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   icon = dirname .. "pics/none_2.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },

   animation_directory = dirname .. "pics",
   animations = { idle = { basename = "none", hotspot = {7, 32}}}
}

tribes:new_immovable_type {
   name = "amazons_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Water"),
   icon = dirname .. "pics/water_2.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },

   animation_directory = dirname .. "pics",
   animations = { idle = { basename = "water", hotspot = {7, 32}}}
}

tribes:new_immovable_type {
   name = "amazons_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Gold"),
   icon = dirname .. "pics/gold_1_2.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },

   animation_directory = dirname .. "pics",
   animations = { idle = { basename = "gold_1", hotspot = {7, 32}}}
}

tribes:new_immovable_type {
   name = "amazons_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Granite"),
   icon = dirname .. "pics/stones_1_2.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },

   animation_directory = dirname .. "pics",
   animations = { idle = { basename = "stones_1", hotspot = {7, 32}}}
}

tribes:new_immovable_type {
   name = "amazons_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Gold"),
   icon = dirname .. "pics/gold_2_2.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },

   animation_directory = dirname .. "pics",
   animations = { idle = { basename = "gold_2", hotspot = {7, 32}}}
}

tribes:new_immovable_type {
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
