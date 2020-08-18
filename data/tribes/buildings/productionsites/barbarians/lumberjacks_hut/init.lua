dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_lumberjacks_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Lumberjack’s Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 43, 45 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 43, 45 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 43, 45 },
      },
   },

   aihints = {
      collects_ware_from_map = "log",
      basic_amount = 1,
   },

   working_positions = {
      barbarians_lumberjack = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"felling trees",
         actions = {
            "callworker=harvest",
            "sleep=duration:20s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Trees",
      heading = _"Out of Trees",
      message = pgettext("barbarians_building", "The lumberjack working at this lumberjack’s hut can’t find any trees in his work area. You should consider dismantling or destroying the building or building a ranger’s hut."),
      productivity_threshold = 60
   },
}
