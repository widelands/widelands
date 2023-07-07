push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "gold_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold Ore"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 4, 4 },
      },
   }
}

pop_textdomain()
