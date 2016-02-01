dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Weaving Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   needs_seafaring = true,

   buildcost = {
      log = 5,
      granite = 2,
      thatch_reed = 2
   },
   return_on_dismantle = {
      log = 2,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 36, 74 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 36, 74 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 36, 74 },
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      barbarians_weaver = 1
   },

   inputs = {
      thatch_reed = 8
   },
   outputs = {
      "cloth"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
         descname = _"weaving",
         actions = {
            "sleep=20000",
            "check_map=seafaring",
            "return=skipped unless economy needs cloth",
            "consume=thatch_reed",
            "animate=working 25000",
            "produce=cloth"
         }
      },
   },
}
