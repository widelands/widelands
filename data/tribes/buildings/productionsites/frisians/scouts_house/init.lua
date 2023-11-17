push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Scout’s House"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      granite = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      granite = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         hotspot = {40, 66},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {40, 60}
      }
   },

   aihints = {},

   working_positions = {
      frisians_scout = 1
   },

   inputs = {
      { name = "ration", amount = 2 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
         descname = _("scouting"),
         actions = {
            -- time of worker: 82.2-214.8 sec
            -- min. time total: 30 + 82.2 = 112.2 sec
            -- max. time total: 30 + 214.8 = 244.8 sec
            -- avg. time total: (112.2 + 244.8) / 2 = 178.5 sec
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
