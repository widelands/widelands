dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_woodcutters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Woodcutter´s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      granite = 1,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 46 },
      },
   },

   aihints = {
      forced_after = 180,
      prohibited_till = 180,
      logproducer = true
   },

   working_positions = {
      frisians_woodcutter = 1
   },

   outputs = {
      "log"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"felling trees",
         actions = {
            "sleep=26000",
            "worker=chop"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Trees",
      heading = _"Out of Trees",
      message = pgettext("frisians_building", "The woodcutter working at this woodcutter´s house can’t find any trees in his work area. You should consider dismantling or destroying the building or building a forester´s house."),
      productivity_threshold = 66
   },
}
