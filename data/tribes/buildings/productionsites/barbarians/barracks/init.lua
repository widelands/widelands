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
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      { name = "ax", amount = 8 }
   },
   outputs = {
      "barbarians_soldier",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = _"recruiting soldier",
         -- TODO(GunChleoc): this should cost us a carrier as well, or maybe a recruit.
         actions = {
            "sleep=15000",
            "return=skipped unless economy needs barbarians_soldier",
            "consume=ax",
            "animate=working 15000",
            "recruit=barbarians_soldier"
         }
      },
   }
}
