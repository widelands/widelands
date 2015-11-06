dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_rangers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ranger’s Hut"),
   directory = dirname,
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
         template = "idle_??",
         directory = dirname,
         hotspot = { 45, 49 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 45, 49 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
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
