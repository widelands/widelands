dirname = "tribes/buildings/productionsites/empire/lumberjacks_house/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_lumberjacks_house1",
   descname = "Lumberjack’s House",
   helptext_script = dirname .. "helptexts.lua",
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

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 40, 59 },
      },
   },

   aihints = {

   },

   working_positions = {
      empire_lumberjack = 1
   },

   outputs = {
      "log"
   },

   programs = {
      work = {
         descname = "felling trees",
         actions = {
            "sleep=400000", -- Barbarian lumberjack sleeps 25000
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
