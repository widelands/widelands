dirname = "tribes/buildings/productionsites/empire/farm/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   name = "empire_farm1",
   descname = "Farm",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
      name = "empire_farm",
      enhancement_cost = {
         planks = 1,
         marble = 1,
         marble_column = 2
      },
      enhancement_return_on_dismantle = {
         planks = 1,
         granite = 1,
         marble = 1,
         marble_column = 1
      }
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

   programs = {
      main = {
         descname = "working",
         actions = {
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         descname = "planting wheat",
         actions = {
            "sleep=duration:14s",
            "callworker=harvest"
         }
      },
      harvest = {
         descname = "harvesting wheat",
         actions = {
            "sleep=duration:4s",
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
