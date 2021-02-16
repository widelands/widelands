dirname = "tribes/buildings/productionsites/barbarians/weaving_mill/"

push_textdomain("scenario_fri03.wmf")

descriptions:new_productionsite_type {
   name = "frisians_dikers_house",
   descname = pgettext("frisians_building", "Diker’s House"),
   icon = dirname .. "menu.png",
   size = "big",
   
   buildcost = {
      log = 5,
      granite = 2,
      reed = 2
   },
   return_on_dismantle = {
      log = 2,
      granite = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 36, 74 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 36, 74 },
      },
   },

   working_positions = {
      frisians_diker = 1
   },
   aihints = {},

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start diking because ...
         descname = _"diking",
         actions = {
            "consume=log:3",
            "callworker=dike",
            "sleep=duration:20s"
         }
      },
   },
}
pop_textdomain()
