push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "brick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Brick"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 7, 6 },
      },
   }
}

pop_textdomain()
