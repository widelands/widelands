push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "bread_paddle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Bread Paddle"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 6, 6 },
      },
   }
}

pop_textdomain()
