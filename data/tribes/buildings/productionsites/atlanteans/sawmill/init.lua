dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_sawmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Sawmill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 3
   },
   return_on_dismantle = {
      log = 1,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 60 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 53, 60 },
         fps = 25
      }
   },

   aihints = {
      basic_amount = 2,
      very_weak_ai_limit = 2,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_sawyer = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },
   outputs = {
      "planks"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
         descname = _"sawing logs",
         actions = {
            "return=skipped unless economy needs planks",
            "consume=log:2",
            "sleep=16500", -- Much faster than barbarians' wood hardener
            "playsound=sound/atlanteans/saw/benchsaw 192",
            "animate=working 20000", -- Much faster than barbarians' wood hardener
            "produce=planks"
         }
      },
   },
}
