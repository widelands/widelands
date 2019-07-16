dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Forester’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {49, 91},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {49, 69},
      },
   },

   aihints = {
      supports_production_of = { "log" },
      space_consumer = true,
   },

   working_positions = {
      frisians_forester = 1
   },

   indicate_workarea_overlaps = {
      frisians_clay_pit = false,
      frisians_berry_farm = false,
      frisians_reed_farm = false,
      frisians_farm = false,
      frisians_foresters_house = false,
      frisians_woodcutters_house = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "callworker=plant",
            "sleep=12000"
         }
      },
   },
}
