push_textdomain("scenario_atl02.wmf")

dirname = "campaigns/atl02.wmf/scripting/tribes/coin_wood/"

tribes:new_ware_type {
   name = "coin_wood",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wood Coin"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
