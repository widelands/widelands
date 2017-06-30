dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "thatch_reed",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Thatch Reed"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      frisians = 4,
      barbarians = 5
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 12 },
      },
   }
}
