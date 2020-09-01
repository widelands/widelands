dirname = "tribes/buildings/productionsites/empire/foresters_house/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   name = "empire_foresters_house1",
   descname = "Forester’s House",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },

   return_on_dismantle = {
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
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
         descname = "planting trees",
         actions = {
            "sleep=duration:1m6s",
            "callworker=plant"
         }
      },
   },
}
pop_textdomain()
