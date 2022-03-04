push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "tabard",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Tabard"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 5, 10 },
      },
   }
}

pop_textdomain()
