dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "custom_ware",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Custom Ware"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      atlanteans = 14,
      barbarians = 14,
      frisians = 4,
      empire = 14
   },

   animations = {
      idle = {
         hotspot = { 6, 7 },
      },
   }
}
