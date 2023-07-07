local dirname = "campaigns/atl02.wmf/scripting/tribes/atlanteans_trading_post/"

push_textdomain("scenario_atl02.wmf")

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_trading_post",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Trading Post"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",


   animations = {
      idle = {
         hotspot = { 60, 59 },
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
      { name = "log", amount = 5 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start trading because ...
         descname = _("trading"),
         actions = {
            "call=trade_log",
            "call=trade_coal",
            "call=trade_iron",
            "call=trade_gold",
         }
      },
      trade_log = {
         -- TRANSLATORS: Completed/Skipped/Did not start trading because ...
         descname = _("trading"),
         actions = {
            "callworker=find_pole",
            "return=failed unless site has log:5",
            "return=skipped unless economy needs coin_wood",
            "consume=log:5",
            "callworker=log_trade",
            "sleep=duration:30s",
         }
      },
      trade_coal = {
         -- TRANSLATORS: Completed/Skipped/Did not start trading because ...
         descname = _("trading"),
         actions = {
            "callworker=find_pole",
            "return=failed unless site has coal:4",
            "return=skipped unless economy needs coin_copper",
            "consume=coal:4",
            "callworker=coal_trade",
            "sleep=duration:30s",
         }
      },
      trade_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start trading because ...
         descname = _("trading"),
         actions = {
            "callworker=find_pole",
            "return=failed unless site has iron:3",
            "return=skipped unless economy needs coin_silver",
            "consume=iron:3",
            "callworker=iron_trade",
            "sleep=duration:30s",
         }
      },
      trade_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start trading because ...
         descname = _("trading"),
         actions = {
            "callworker=find_pole",
            "return=failed unless site has gold:2",
            "return=skipped unless economy needs coin_gold",
            "consume=gold:2",
            "callworker=gold_trade",
            "sleep=duration:30s",
         }
      },
   },
}

pop_textdomain()
