push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "ration",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Ration"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 3, 5 },
      },
   }
}

pop_textdomain()
