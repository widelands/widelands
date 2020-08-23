dirname = path.dirname(__file__)


tribes:new_ware_type {
   msgctxt = "ware",
   name = "cassavaroot",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cassavaroot"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      amazons = 10
   },

   animations = {
      idle = {
         hotspot = { 6, 6 },
      }
   },
}
