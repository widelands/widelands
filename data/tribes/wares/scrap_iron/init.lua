push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "scrap_iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Scrap Iron"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 9 },
      },
   }
}

pop_textdomain()
