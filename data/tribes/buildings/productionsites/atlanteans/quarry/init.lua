push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "atlanteans_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Quarry"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      planks = 1
   },
   return_on_dismantle = {
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 40, 45 },
      },
   },

   aihints = {
      collects_ware_from_map = "granite"
   },

   working_positions = {
      atlanteans_stonecutter = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"quarrying granite",
         actions = {
            "callworker=cut_granite",
            "sleep=duration:18s500ms"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("atlanteans_building", "The stonecutter working at this quarry can’t find any rocks in his work area."),
      productivity_threshold = 75
   },
}

pop_textdomain()
