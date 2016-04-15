dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "wheat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wheat"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      barbarians = 12,
      empire = 12
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { -1, 6 },
      },
   }
}
