dirname = "campaigns/atl02.wmf/scripting/tribes/atlantean_trading_post/"

push_textdomain("scenario_atl02.wmf")

tribes:new_productionsite_type {
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

   working_positions = {
      atlanteans_trader = 1
   },

   inputs = {
      { name = "gold", amount = 2 },
      { name = "iron", amount = 3 },
      { name = "coal", amount = 4 },
      { name = "log", amount = 5 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start trading because ...
         descname = _"trading",
         actions = {
            "callworker=find_pole",
            "consume=log:5",
            "callworker=trade",
            "sleep=duration:30s",
            "consume=coal:4",
            "sleep=duration:30s",
            "callworker=trade"
         }
      },
   },
}
