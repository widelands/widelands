dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_collectors_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Fruit Collector´s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 3,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 2,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 43, 45 },
      }
   },

   aihints = {
      forced_after = 180,
      prohibited_till = 180
   },

   working_positions = {
      frisians_fruit_collector = 1
   },

   outputs = {
      "fruit"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"working",
         actions = {
            "sleep=18000",
            "worker=harvest",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fruit",
      heading = _"Out of Berries",
      message = pgettext("frisians_building", "The fruit collector working at this collector´s house can’t find any berries in his work area. You should consider dismantling or destroying the building or building a berry farm."),
      productivity_threshold = 66
   },
}
