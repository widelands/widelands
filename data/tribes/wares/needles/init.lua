push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "needles",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Needles"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 4, 3 },
      },
   }
}

pop_textdomain()
