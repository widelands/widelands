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
      forced_after = 180,
      prohibited_till = 180,
      logproducer = true
   },

   working_positions = {
      barbarians_lumberjack = 1
   },

   outputs = {
      "log"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"felling trees",
         actions = {
            "sleep=25000", -- Sleeps shorter than any other tribes.
            "worker=chop"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Trees",
      heading = _"Out of Trees",
      message = pgettext("barbarians_building", "The lumberjack working at this lumberjack’s hut can’t find any trees in his work area. You should consider dismantling or destroying the building or building a ranger’s hut."),
      productivity_threshold = 66
   },
}
