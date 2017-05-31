dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Barracks"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 4,
      granite = 2,
      grout = 4,
      thatch_reed = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 1,
      grout = 1
   },

   animations = {
      idle = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 64 },
      },
      working = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 64 },
      }
   },

   aihints = {
      is_basic = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      { name = "ax", amount = 8 },
      { name = "barbarians_recruit", amount = 8 }
   },
   outputs = {
      "barbarians_soldier",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("barbarians_building", "recruiting soldier"),
         actions = {
            "sleep=15000",
            "return=skipped unless economy needs barbarians_soldier",
            "consume=ax barbarians_recruit",
            "animate=working 15000",
            "recruit=barbarians_soldier"
         }
      },
   }
}
