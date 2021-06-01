push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "cornmeal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cornmeal"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 4, 15 },
      },
   }
}

pop_textdomain()
