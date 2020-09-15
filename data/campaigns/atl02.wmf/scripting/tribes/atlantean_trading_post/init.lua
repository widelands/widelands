dirname = "campaigns/atl02.wmf/scripting/tribes/atlantean_trading_post/"

push_textdomain("scenario_atl02.wmf")

tribes:new_market_type {
   name = "atlanteans_trading_post",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Trading Post"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",


   animations = {
      idle = {
         hotspot = { 50, 56 },
      },
   },
   aihints = {},

   carrier = "atlanteans_horse",
}
