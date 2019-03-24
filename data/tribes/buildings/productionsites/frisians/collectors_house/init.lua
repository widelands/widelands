dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_collectors_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Fruit Collector’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {49, 89},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {49, 67},
      },
   },

   aihints = {
      collects_ware_from_map = "fruit",
      prohibited_till = 470,
      requires_supporters = true
   },

   working_positions = {
      frisians_fruit_collector = 1
   },

   outputs = {
      "fruit"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start gathering berries because ...
         descname = _"gathering berries",
         actions = {
            "sleep=21000",
            "callworker=harvest",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Bushes",
      heading = _"Out of Berries",
      message = pgettext ("frisians_building", "The fruit collector working at this fruit collector’s house can’t find any berry bushes in his work area. You should consider building another berry farm, or dismantling or destroying this building."),
      productivity_threshold = 8
   },
}
