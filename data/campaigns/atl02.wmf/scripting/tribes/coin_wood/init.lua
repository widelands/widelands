push_textdomain("scenario_atl02.wmf")

local dirname = "campaigns/atl02.wmf/scripting/tribes/coin_wood/"

wl.Descriptions():new_ware_type {
   name = "coin_wood",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wood Coin"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
