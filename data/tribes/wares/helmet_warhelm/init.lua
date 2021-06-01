push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "helmet_warhelm",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Warhelm"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 7, 12 },
      },
   }
}

pop_textdomain()
