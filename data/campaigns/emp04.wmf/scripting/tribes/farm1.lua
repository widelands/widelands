dirname = "tribes/buildings/productionsites/empire/farm/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_farm1",
   descname = "Farm",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   enhancement = "empire_farm2",

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
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         descname = "planting wheat",
         actions = {
            "sleep=14000",
            "callworker=harvest"
         }
      },
      harvest = {
         descname = "harvesting wheat",
         actions = {
            "sleep=4000",
            "callworker=harvest"
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
pop_textdomain()
