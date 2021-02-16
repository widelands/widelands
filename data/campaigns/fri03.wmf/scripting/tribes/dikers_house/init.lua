-- TODO(Nordfriese): Create graphics
dirname = "tribes/buildings/productionsites/barbarians/weaving_mill/"

push_textdomain("scenario_fri03.wmf")

descriptions:new_productionsite_type {
   name = "frisians_dikers_house",
   descname = pgettext("frisians_building", "Diker’s House"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 2,
      log = 5,
      granite = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      reed = 1,
      log = 2,
      granite = 1
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
            "sleep=duration:30s"
         }
      },
   },
}
pop_textdomain()
