push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Scout’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 53 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 50, 53 },
      },
   },

   aihints = {},

   working_positions = {
      empire_scout = 1
   },

   inputs = {
      { name = "ration", amount = 2 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
         descname = _"scouting",
         actions = {
            "consume=ration",
            "sleep=duration:30s",
            "callworker=scout"
         }
      },
   },
}

pop_textdomain()
