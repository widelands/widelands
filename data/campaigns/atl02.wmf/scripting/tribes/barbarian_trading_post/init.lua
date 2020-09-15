dirname = "campaigns/atl02.wmf/scripting/tribes/barbarian_trading_post/"

push_textdomain("scenario_atl02.wmf")

tribes:new_market_type {
   name = "barbarians_trading_post",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Trading Post"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",


   animations = {
      idle = {
         hotspot = { 64, 106 },
      },
   },
   aihints = {},
   
   carrier = "barbarians_ox",
}
