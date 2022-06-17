push_textdomain("scenario_atl02.wmf")

dirname = "campaigns/atl02.wmf/scripting/tribes/coin_gold/"

wl.Descriptions():new_ware_type {
   name = "coin_gold",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold Coin"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
