push_textdomain("scenario_atl02.wmf")

dirname = "campaigns/atl02.wmf/scripting/tribes/atlanteans_scouts_house1/"

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_scouts_house1",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Scout’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   enhancement = {
      name = "atlanteans_trading_post",
      enhancement_cost = {
         planks = 4,
         granite = 2,
         diamond = 1,
         spidercloth = 1,
      },
      enhancement_return_on_dismantle = {
         planks = 2,
         granite = 1,
         spidercloth = 1
      }
   },

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
         hotspot = { 44, 49 },
      },
   },

   spritesheets = {
      build = {
         frames = 3,
         columns = 3,
         rows = 1,
         hotspot = { 44, 49 },
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
