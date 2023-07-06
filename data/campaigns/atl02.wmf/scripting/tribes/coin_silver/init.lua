push_textdomain("scenario_atl02.wmf")

local dirname = "campaigns/atl02.wmf/scripting/tribes/coin_silver/"

wl.Descriptions():new_ware_type {
   name = "coin_silver",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Silver Coin"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
