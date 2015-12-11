dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_rangers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ranger’s Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4
   },
   return_on_dismantle = {
      log = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 45, 49 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 45, 49 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 45, 49 },
      },
   },

   aihints = {
      renews_map_resource = "log",
      space_consumer = true,
      prohibited_till = 200
   },

   working_positions = {
      barbarians_ranger = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "sleep=16000",
            "worker=plant"
         }
      },
   },
}
