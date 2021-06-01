push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "honey",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Honey"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 4, 5 },
      },
   }
}

pop_textdomain()
