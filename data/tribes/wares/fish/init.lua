push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fish"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 8, 6 },
      },
   }
}

pop_textdomain()
