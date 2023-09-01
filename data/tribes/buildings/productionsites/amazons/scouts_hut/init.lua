push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_scouts_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Scout’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      granite = 1,
      log = 2,
      rope = 1
   },
   return_on_dismantle = {
      granite = 1,
      log = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 57}},
      unoccupied = {hotspot = {43, 57}}
   },

   aihints = {},

   working_positions = {
      amazons_scout = 1
   },

   inputs = {
      { name = "ration", amount = 2 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
         descname = _("scouting"),
         actions = {
            -- time of worker: 75 sec
            -- radius: 15
            -- steps from building to beginning of scouting: 2...18
            -- min. time total: 2 * 1.8 + 75 + 2 * 1.8 + 30 = 112.2 sec
            -- max. time total: 2 * (18 * 1.8 + 75) + 30 = 244.8 sec
            "consume=ration",
            "callworker=scout",
            "sleep=duration:30s"
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
