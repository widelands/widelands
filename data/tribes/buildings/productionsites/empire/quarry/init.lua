push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Quarry"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      planks = 1
   },
   return_on_dismantle = {
      log = 1
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 40, 57 },
      },
   },

   aihints = {},

   working_positions = {
      empire_stonemason = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- min. time total: 4 * 42.2 = 168.8 sec
            -- max. time total: 4 * 67.4 = 269.6 sec
            -- avg. time total: 4 * 54.8 = 219.2 sec
            "call=mine_granite on failure fail",
            "call=mine_granite on failure fail",
            "call=mine_marble on failure fail", -- This will find marble 1 out of 4 times
            "call=mine_granite on failure fail",
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _("quarrying granite"),
         actions = {
            -- time of worker: 24.7-49.9 sec, min+max average 37.3 sec
            -- min. time: 24.7 + 17.5 = 42.2 sec
            -- max. time: 49.9 + 17.5 = 67.4 sec
            -- avg. time: 37.3 + 17.5 = 54.8 sec
            "callworker=cut_granite",
            "sleep=duration:17s500ms"
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying marble because ...
         descname = _("quarrying marble"),
         actions = {
            -- time of worker: 24.7-49.9 sec, min+max average 37.3 sec
            -- min. time: 24.7 + 17.5 = 42.2 sec
            -- max. time: 49.9 + 17.5 = 67.4 sec
            -- avg. time: 37.3 + 17.5 = 54.8 sec
            "callworker=cut_marble",
            "sleep=duration:17s500ms"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Rocks"),
      heading = _("Out of Rocks"),
      message = pgettext("empire_building", "The stonemason working at this quarry can’t find any rocks in his work area."),
      productivity_threshold = 75
   },
}

pop_textdomain()
