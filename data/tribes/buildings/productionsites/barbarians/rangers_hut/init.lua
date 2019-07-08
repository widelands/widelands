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
      supports_production_of = { "log" },
      space_consumer = true,
      basic_amount = 1,
      very_weak_ai_limit = 3,
      weak_ai_limit = 5
   },

   working_positions = {
      barbarians_ranger = 1
   },

   indicate_workarea_overlaps = {
      barbarians_rangers_hut = false,
      barbarians_lumberjacks_hut = true,
      barbarians_farm = false,
      barbarians_reed_yard = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "callworker=plant",
            "sleep=11500"
         }
      },
   },
}
