dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "barbarians_ware",
   name = "barbarians_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("barbarians_ware", "Pitta Bread"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 20
   },
   preciousness = {
      barbarians = 4
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 6 },
      },
   }
}
