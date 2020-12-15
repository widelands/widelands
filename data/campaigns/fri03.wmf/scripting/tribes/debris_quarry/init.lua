dirname = "tribes/buildings/productionsites/frisians/quarry/"

push_textdomain("scenario_fri03.wmf")

descriptions:new_productionsite_type {
   name = "frisians_debris_quarry",
   descname = pgettext("frisians_building", "Debris Quarry"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      granite = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {40, 71},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = { unoccupied = { hotspot = {40, 53}}},

   working_positions = {
      frisians_debris_stonemason = 1
   },
   aihints = {},

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start removing debris because ...
         descname = _"removing debris",
         actions = {
            "callworker=quarry",
            "sleep=duration:20s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Debris",
      heading = _"Out of Debris",
      message = pgettext("frisians_building", "The stonemason working at this debris quarry has removed all the debris in his work area."),
      productivity_threshold = 75
   },
}
pop_textdomain()
