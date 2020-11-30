push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_ferry_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Ferry Yard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"waterways"},

   buildcost = {
      planks = 3,
      granite = 3,
      spidercloth = 2,
      quartz = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 2,
      spidercloth = 1,
      quartz = 1
   },

   -- TODO(Nordfriese): Make animations
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 66 },
      }
   },

   aihints = {
      --needs_water = true,
      --shipyard = true,
      prohibited_till = 3600
   },

   working_positions = {
      europeans_shipwright = 1
   },

   inputs = {
      { name = "planks", amount = 6 },
      { name = "spidercloth", amount = 3 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=duration:20s",
            "callworker=buildferry_1",
            "consume=planks:2 spidercloth",
            "callworker=buildferry_2"
         }
      },
   },
}

pop_textdomain()
