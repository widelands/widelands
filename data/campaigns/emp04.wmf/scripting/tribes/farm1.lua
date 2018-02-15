dirname = "tribes/buildings/productionsites/empire/farm/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_farm1",
   descname = "Farm",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",


   return_on_dismantle = {
      planks = 1,
      granite = 1,
      marble = 1,
      marble_column = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 82, 74 },
      },
   },

   aihints = {

   },

   working_positions = {
      empire_farmer = 1
   },

   outputs = {
      "wheat"
   },

   programs = {
      work = {
         descname = "working",
         actions = {
            "call=plant_wheat",
            "call=harvest_wheat",
            "return=skipped"
         }
      },
      plant_wheat = {
         descname = "planting wheat",
         actions = {
            "sleep=14000",
            "worker=harvest"
         }
      },
      harvest_wheat = {
         descname = "harvesting wheat",
         actions = {
            "sleep=4000",
            "worker=harvest"
         }
      },
   },
   out_of_resource_notification = {
      title = "No Fields",
      heading = "Out of Fields",
      message = "The farmer working at this farm has no cleared soil to plant his seeds.",
      productivity_threshold = 30
   },
}
