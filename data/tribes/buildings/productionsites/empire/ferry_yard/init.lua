dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_ferry_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Ferry Yard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"waterways"},

   buildcost = {
      log = 3,
      planks = 2,
      granite = 3,
      cloth = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      cloth = 1
   },

   -- TODO(Nordfriese): Make animations
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 63 },
      }
   },

   aihints = {},

   indicate_workarea_overlaps = {
      empire_ferry_yard = false,
   },

   working_positions = {
      empire_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 8 },
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=20000",
            "callworker=buildferry_1",
            "consume=log:3",
            "callworker=buildferry_2"
         }
      },
   },
}
