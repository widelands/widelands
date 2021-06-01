push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "saw",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Saw"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 7, 5 },
      },
   }
}

pop_textdomain()
