dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "sword_short",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Short Sword"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 30
   },
   preciousness = {
      frisians = 4
   },

   animations = {
      idle = {
         hotspot = { 12, 9 },
      }
   },
}
