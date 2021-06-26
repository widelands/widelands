dirname = "tribes/buildings/productionsites/empire/foresters_house/"

push_textdomain("scenario_emp04.wmf")

descriptions:new_productionsite_type {
   name = "empire_foresters_house1",
   descname = pgettext("empire_building", "Forester’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },

   return_on_dismantle = {
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 52, 54 },
      },
   },

   aihints = {
   },

   working_positions = {
      empire_forester = 1
   },

   programs = {
      main = {
         descname = pgettext("empire_building", "planting trees"),
         actions = {
            "sleep=duration:1m6s",
            "callworker=plant"
         }
      },
   },
}
pop_textdomain()
