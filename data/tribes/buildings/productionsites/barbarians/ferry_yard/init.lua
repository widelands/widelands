push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_ferry_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ferry Yard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"waterways"},

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 3,
      cloth = 2
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 2
   },

   -- TODO(Nordfriese): Make animations
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 62, 48 },
      }
   },

   aihints = {},

   working_positions = {
      barbarians_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 6 },
      { name = "cloth", amount = 3 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
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
