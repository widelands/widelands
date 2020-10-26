dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

-- Test that replacing a ware works

descriptions:new_ware_type {
   name = "ax_sharp",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Replaced Ware"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 1
   },
   preciousness = {
      barbarians = 1
   },

   animations = {
      idle = {
         hotspot = { 7, 7 },
      },
   }
}
