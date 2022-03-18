push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "cloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cloth"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 6, 9 },
      },
   }
}

pop_textdomain()
