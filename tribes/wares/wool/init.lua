dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "wool",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wool"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 10
   },
   preciousness = {
      empire = 2
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 8, 18 },
      },
   }
}
