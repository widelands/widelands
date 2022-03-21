push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Scout’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 41, 44 },
      },
   },

   spritesheets = {
      build = {
         frames = 3,
         rows = 3,
         columns = 1,
         hotspot = { 40, 44 }
      },
   },

   aihints = {},

   working_positions = {
      atlanteans_scout = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 2 },
      { name = "atlanteans_bread", amount = 2 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
         descname = _("scouting"),
         actions = {
            "consume=smoked_fish",
            "sleep=duration:30s",
            "callworker=scout",
            "consume=atlanteans_bread",
            "sleep=duration:30s",
            "callworker=scout"
         }
      },
      targeted_scouting = {
         descname = _("scouting"),
         actions = {
            "consume=smoked_fish,atlanteans_bread",
            "callworker=targeted_scouting"
         }
      },
   },
}

pop_textdomain()
