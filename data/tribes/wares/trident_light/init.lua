push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "trident_light",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Light Trident"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
