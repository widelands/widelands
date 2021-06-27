dirname = "tribes/buildings/productionsites/empire/farm/"

push_textdomain("scenario_emp04.wmf")

descriptions:new_productionsite_type {
   name = "empire_farm1",
   descname = pgettext("empire_building", "Farm"),
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

   animation_directory = dirname,
   animations = {
      idle = {
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
         descname = pgettext("empire_building", "working"),
         actions = {
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         descname = pgettext("empire_building", "planting wheat"),
         actions = {
            "sleep=duration:14s",
            "callworker=harvest"
         }
      },
      harvest = {
         descname = pgettext("empire_building", "harvesting wheat"),
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
