push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "corn",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Corn"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 6, 11 },
      },
   }
}

pop_textdomain()
