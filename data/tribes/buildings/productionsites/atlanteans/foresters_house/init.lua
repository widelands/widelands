dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Forester’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },
   return_on_dismantle = {
      granite = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 37, 44 },
      }
   },

   aihints = {
      space_consumer = true,
      supports_production_of = { "log" },
      very_weak_ai_limit = 2,
      weak_ai_limit = 4
   },

   working_positions = {
      atlanteans_forester = 1
   },

   indicate_workarea_overlaps = {
      atlanteans_blackroot_farm = false,
      atlanteans_farm = false,
      atlanteans_foresters_house = false,
      atlanteans_woodcutters_house = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "callworker=plant",
            "sleep=8000"
         }
      },
   },
}
