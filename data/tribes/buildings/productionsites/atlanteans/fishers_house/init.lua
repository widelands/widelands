push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Fisher’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 2
   },
   return_on_dismantle = {
      planks = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 40, 50 },
      },
   },

   aihints = {
      needs_water = true,
      basic_amount = 1,
      prohibited_till = 540,
      very_weak_ai_limit = 2,
      weak_ai_limit = 4
   },

   working_positions = {
      atlanteans_fisher = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _("fishing"),
         actions = {
            -- time of worker: 9.5 sec
            -- radius: 7
            -- steps from building to water: 2...10
            -- min. time total: 2 * 2 * 1.8 + 9.5 + 9.5 = 26.2 sec
            -- max. time total: 2 * 10 * 1.8 + 9.5 + 9.5 = 55 sec
            "callworker=fish",
            "sleep=duration:9s500ms"
         }
      },

   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fish"),
      heading = _("Out of Fish"),
      message = pgettext("atlanteans_building", "The fisher working out of this fisher’s house can’t find any fish in his work area. Remember that you can increase the number of existing fish by building a fish breeder’s house."),
   },
}

pop_textdomain()
