push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "fishing_net",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fishing Net"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
