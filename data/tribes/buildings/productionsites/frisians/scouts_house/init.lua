dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Scout’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
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
         directory = dirname,
         basename = "idle",
         hotspot = {40, 66},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
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
         descname = _"scouting",
         actions = {
            "consume=ration",
            "sleep=duration:30s",
            "callworker=scout"
         }
      },
   },
}
