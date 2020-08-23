dirname = path.dirname(__file__)


tribes:new_ware_type {
   msgctxt = "ware",
   name = "stonebowl",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Stonebowl"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      amazons = 1
   },
   preciousness = {
      amazons = 1
   },

   animations = {
      idle = {
         hotspot = { 6, 6 },
      }
   },
}
