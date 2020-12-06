push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_lumberjacks_house_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Lumberjack’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",
   
   enhancement = {
        name = "europeans_lumberjacks_house_normal",
        enhancement_cost = {
          brick = 1,
          grout = 1,
          spidercloth = 1,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 1,
          quartz = 1,
        },
   },

   buildcost = {
      planks = 2,
      reed = 2
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
      collects_ware_from_map = "log",
      basic_amount = 1,
   },

   working_positions = {
      europeans_lumberjack_basic = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"felling trees",
         actions = {
            "callworker=harvest",
            "sleep=duration:18s",
            "produce=log"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Trees",
      heading = _"Out of Trees",
      message = pgettext("europeans_building", "The lumberjack working at this lumberjack’s hut can’t find any trees in his work area. You should consider dismantling or destroying the building or building a ranger’s hut."),
      productivity_threshold = 60
   },
}

pop_textdomain()
