push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "machete",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Machete"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 2 },
      },
   }
}

pop_textdomain()
