push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_ferry_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Ferry Yard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"waterways"},

   buildcost = {
      log = 3,
      planks = 2,
      granite = 3,
      cloth = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      cloth = 1
   },

   -- TODO(Nordfriese): Make animations
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 63 },
      }
   },

   aihints = {},

   working_positions = {
      empire_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 6 },
      { name = "cloth", amount = 3 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=duration:20s",
            "callworker=buildferry_1",
            "consume=log:2 cloth",
            "callworker=buildferry_2"
         }
      },
   },
}

pop_textdomain()
