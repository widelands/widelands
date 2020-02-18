dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_custom_building",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("custom_building", "Custom Building"),
   helptext_script = dirname .. "helptexts.lua",
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 43, 45 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 43, 45 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 43, 45 },
      },
   },

   aihints = {
      collects_ware_from_map = "custom_ware",
      basic_amount = 1,
   },

   working_positions = {
      barbarians_custom_worker = 1
   },

   outputs = {
      "custom_ware"
   },

   indicate_workarea_overlaps = {
      barbarians_rangers_hut = true,
      barbarians_lumberjacks_hut = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"felling trees",
         actions = {
            "callworker=harvest",
            "sleep=20000"
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
