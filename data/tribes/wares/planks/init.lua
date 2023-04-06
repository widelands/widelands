push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "planks",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Planks"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 12, 17 },
      },
   }
}

pop_textdomain()
