push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "needles",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Needles"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 4, 3 },
      },
   }
}

pop_textdomain()
