push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "amazons_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Bread"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = {7, 6},
      },
   }
}

pop_textdomain()
