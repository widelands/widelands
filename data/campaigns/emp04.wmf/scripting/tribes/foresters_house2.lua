dirname = "tribes/buildings/productionsites/empire/foresters_house/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_foresters_house2",
   descname = "Forester’s House",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   enhancement_cost = {
      log = 2,
      planks = 1
   },

   return_on_dismantle_on_enhanced = {
      planks = 0
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
      work = {
         descname = "planting trees",
         actions = {
            "sleep=66000",
            "callworker=plant"
         }
      },
   },
}
