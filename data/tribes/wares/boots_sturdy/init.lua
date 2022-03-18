push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "boots_sturdy",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Sturdy Boots"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 9, 13 },
      },
   }
}

pop_textdomain()
