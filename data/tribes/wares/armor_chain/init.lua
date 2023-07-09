push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "armor_chain",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Chain Armor"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 3, 11 },
      },
   }
}

pop_textdomain()
