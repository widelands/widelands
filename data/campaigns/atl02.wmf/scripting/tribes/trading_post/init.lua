dirname = "campaigns/atl02.wmf/scripting/tribes/trading_post/"

push_textdomain("scenario_atl02.wmf")

tribes:new_market_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_trading_post",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Trading Post"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",


   animations = {
      idle = {
         hotspot = { 67, 80 },
      },
   },
   aihints = {},
   
   carrier = "atlanteans_horse",
}
