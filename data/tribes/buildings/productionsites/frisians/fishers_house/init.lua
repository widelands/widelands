push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Fisher’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {40, 70},
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
         hotspot = {40, 52}
      }
   },

   aihints = {
      needs_water = true,
      prohibited_till = 490
   },

   working_positions = {
      frisians_fisher = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _("fishing"),
         actions = {
            "callworker=fish",
            "sleep=duration:16s" -- TODO(stonerl): reduce to 3s and move 13s to the worker
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fish"),
      heading = _("Out of Fish"),
      message = pgettext ("frisians_building", "The fisher working out of this fisher’s house can’t find any fish in his work area."),
   },
}

pop_textdomain()
