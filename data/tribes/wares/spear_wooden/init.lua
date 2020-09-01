push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "spear_wooden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wooden Spear"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 11 },
      },
   }
}

pop_textdomain()
