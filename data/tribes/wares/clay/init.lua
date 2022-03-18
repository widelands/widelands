push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "clay",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Clay"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 7, 10 },
      },
   }
}

pop_textdomain()
