push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "armor_gilded",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gilded Armor"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 4, 11 },
      },
   }
}

pop_textdomain()
