dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Barracks"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 2,
      granite = 3,
      log = 5,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      granite = 2,
      log = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 57 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 57 },
      }
   },

   aihints = {
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "sword_basic", amount = 8 },
      { name = "fur_clothes", amount = 8 },
      { name = "frisians_carrier", amount = 8 }
   },
   outputs = {
      "frisians_soldier",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("frisians_building", "recruiting soldier"),
         actions = {
            "sleep=10000",
            "return=skipped unless economy needs frisians_soldier",
            "consume=sword_basic fur_clothes frisians_carrier",
            "animate=working 20000",
            "recruit=frisians_soldier"
         }
      },
   }
}
