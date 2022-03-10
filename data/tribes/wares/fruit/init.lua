push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "fruit",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fruit"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 4, 7 },
      }
   },
}

pop_textdomain()
