dirname = "tribes/buildings/productionsites/empire/lumberjacks_house/"

push_textdomain("scenario_emp04.wmf")

descriptions:new_productionsite_type {
   name = "empire_lumberjacks_house1",
   descname = pgettext("empire_building", "Lumberjack’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      planks = 1
   },

   return_on_dismantle = {
      log = 1,
      planks = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 40, 59 },
      },
   },

   aihints = {
   },

   working_positions = {
      empire_lumberjack = 1
   },

   programs = {
      main = {
         descname = pgettext("empire_building", "felling trees"),
         actions = {
            "sleep=duration:6m40s",
            "callworker=harvest"
         }
      },
   },
   out_of_resource_notification = {
      title = "No Trees",
      heading = "Out of Trees",
      message = "The lumberjack working at this lumberjack’s house can’t find any trees in his work area. You should consider dismantling or destroying the building or building a forester’s house.",
      productivity_threshold = 60
   },
}
pop_textdomain()
