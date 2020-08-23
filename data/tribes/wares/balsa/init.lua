dirname = path.dirname(__file__)


tribes:new_ware_type {
   msgctxt = "ware",
   name = "balsa",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Balsa"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      amazons = 20,
   },
   preciousness = {
      amazons = 20,
   },

   animations = {
      idle = {
         hotspot = { 8, 8 },
      }
   },
}
