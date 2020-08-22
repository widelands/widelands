dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Mill"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      granite = 3,
      marble = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      marble = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 87 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 41, 87 },
         fps = 25
      },
   },

   aihints = {
      prohibited_till = 540
   },

   working_positions = {
      empire_miller = 1
   },

   inputs = {
      { name = "wheat", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding wheat because ...
         descname = _"grinding wheat",
         actions = {
            "return=skipped unless economy needs flour",
            "consume=wheat",
            "sleep=duration:5s",
            "playsound=sound/mill/mill_turning priority:90% allow_multiple",
            "animate=working duration:10s",
            "produce=flour"
         }
      },
   },
}
