push_textdomain("tribes")

dirname = path.dirname (__file__)

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
