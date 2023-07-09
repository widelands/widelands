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
            "call=mine_granite on failure fail",
            "call=mine_granite on failure fail",
            "call=mine_granite on failure fail",
            "call=mine_marble on failure fail", -- This will find marble 2 out of 7 times
            "call=mine_granite on failure fail",
            "call=mine_granite on failure fail",
            "call=mine_marble on failure fail", -- This will find marble 2 out of 7 times
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _("quarrying granite"),
         actions = {
            "callworker=cut_granite",
            "sleep=duration:17s500ms"
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying marble because ...
         descname = _("quarrying marble"),
         actions = {
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
