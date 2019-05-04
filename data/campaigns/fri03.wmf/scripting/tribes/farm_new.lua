dirname = "tribes/buildings/productionsites/frisians/farm/"

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_farm_new",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {105, 138},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {105, 138},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {105, 111},
      },
   },

   aihints = {},

   working_positions = {
      frisians_farmer = 1
   },

   outputs = {
      "barley"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_barley",
            "call=harvest_barley",
            "return=no_stats"
         }
      },
      plant_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting barley because ...
         descname = _"planting barley",
         actions = {
            "sleep=18000",
            "callworker=plant"
         }
      },
      harvest_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting barley because ...
         descname = _"harvesting barley",
         actions = {
            "sleep=8000",
            "callworker=harvest",
            "animate=working 40000",
            "produce=barley" --produces 2 barley per field
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fields",
      heading = _"Out of Fields",
      message = pgettext ("frisians_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}
