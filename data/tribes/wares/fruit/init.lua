push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "fruit",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fruit"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 4, 7 },
      }
   },
}

pop_textdomain()
