dirname = "tribes/buildings/productionsites/empire/foresters_house/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_foresters_house1",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Forester’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   enhancement = "empire_foresters_house2",
   
   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },
   
   return_on_dismantle = {
      planks = 0,
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
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "sleep=66000",
            "worker=plant"
         }
      },
   },
}
