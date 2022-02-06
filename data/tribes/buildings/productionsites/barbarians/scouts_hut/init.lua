push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_scouts_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Scout’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {
         hotspot = { 45, 92 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 44, 92 }
      },
      idle = {
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 44, 92 }
      },
   },

   aihints = {},

   working_positions = {
      barbarians_scout = 1
   },

   inputs = {
      { name = "ration", amount = 2 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
         descname = _("scouting"),
         actions = {
            "consume=ration",
            "sleep=duration:30s",
            "callworker=scout"
         }
      },
      targeted_scouting = {
         descname = _("scouting"),
         actions = {
            "consume=ration",
            "callworker=targeted_scouting"
         }
      },
   },
}

pop_textdomain()
