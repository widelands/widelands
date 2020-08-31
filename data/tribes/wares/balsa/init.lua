push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "balsa",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Balsa"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 8, 8 },
      }
   },
}

pop_textdomain()
