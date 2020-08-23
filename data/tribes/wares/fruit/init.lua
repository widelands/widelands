dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fruit",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fruit"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      frisians = 1
   },

   animations = {
      idle = {
         hotspot = { 4, 7 },
      }
   },
}
