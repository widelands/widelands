push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "fire_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fire Tongs"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 5, 5 },
      },
   }
}

pop_textdomain()
