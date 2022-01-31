dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_custom_building",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("custom_building", "Custom Building"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         hotspot = { 43, 45 },
      },
      build = {
         hotspot = { 43, 45 },
      },
      unoccupied = {
         hotspot = { 43, 45 },
      },
   },

   aihints = {
      basic_amount = 1,
   },

   working_positions = {
      barbarians_custom_worker = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _("felling trees"),
         actions = {
            "callworker=harvest",
            "sleep=duration:20s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = "No Trees",
      heading = "Out of Trees",
      message = "Custom out of resource message.",
      productivity_threshold = 60
   },
}
