push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Quarry"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4
   },
   return_on_dismantle = {
      log = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 45, 48 },
      },
      unoccupied = {
         hotspot = { 45, 48 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 45, 48 }
      },
   },

   aihints = {},

   working_positions = {
      barbarians_stonemason = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _("quarrying granite"),
         actions = {
            -- time of worker: 24.7-49.9 sec, min+max average 37.3 sec
            -- min. time total: 24.7 + 17.5 = 42.2 sec
            -- max. time total: 49.9 + 17.5 = 67.4 sec
            -- avg. time total: 37.3 + 17.5 = 54.8 sec
            "callworker=cut_granite",
            "sleep=duration:17s500ms"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Rocks"),
      heading = _("Out of Rocks"),
      message = pgettext("barbarians_building", "The stonemason working at this quarry can’t find any rocks in his work area."),
      productivity_threshold = 75
   },
}

pop_textdomain()
