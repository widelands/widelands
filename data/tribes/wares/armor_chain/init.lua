push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "armor_chain",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Chain Armor"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 3, 11 },
      },
   }
}

pop_textdomain()
