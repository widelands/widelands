push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "trident_steel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Steel Trident"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
