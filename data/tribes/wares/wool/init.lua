push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "wool",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wool"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 8, 18 },
      },
   }
}

pop_textdomain()
