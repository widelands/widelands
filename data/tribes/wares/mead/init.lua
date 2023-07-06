push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "mead",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mead"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 5, 9 },
      },
   }
}

pop_textdomain()
