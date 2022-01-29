push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "atlanteans_ferry_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Ferry Yard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"waterways"},

   buildcost = {
      log = 3,
      planks = 2,
      granite = 3,
      spidercloth = 2
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 2,
      spidercloth = 1
   },

   -- TODO(Nordfriese): Make animations
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 53, 66 },
      }
   },

   aihints = {},

   working_positions = {
      atlanteans_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 6 },
      { name = "spidercloth", amount = 3 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "sleep=duration:20s",
            "callworker=buildferry_1",
            "consume=log:2 spidercloth",
            "callworker=buildferry_2"
         }
      },
   },
}

pop_textdomain()
