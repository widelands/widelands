push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Forester’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 54 },
      },
   },

   aihints = {
      space_consumer = true,
      supports_production_of = { "log" },
      very_weak_ai_limit = 2,
      weak_ai_limit = 4
   },

   working_positions = {
      empire_forester = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
   },
}

pop_textdomain()
