push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "cornmeal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cornmeal"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 4, 15 },
      },
   }
}

pop_textdomain()
