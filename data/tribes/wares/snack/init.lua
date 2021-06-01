push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "snack",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Snack"),
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
