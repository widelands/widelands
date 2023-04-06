dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

-- Test that replacing a ware works

wl.Descriptions():new_ware_type {
   name = "ax_sharp",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Replaced Ware"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 7, 7 },
      },
   }
}
