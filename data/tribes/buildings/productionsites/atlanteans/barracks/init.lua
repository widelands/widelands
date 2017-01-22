dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Barracks"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 2,
      spidercloth = 3,
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      planks = 1,
      spidercloth = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 65 },
      },
      working = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 65 },
      }
   },

   aihints = {
      --forced_after = 1000,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      atlanteans_trainer = 1
   },

   inputs = {
      { name = "tabard", amount = 8 },
      { name = "trident_light", amount = 8 },
      { name = "atlanteans_recruit", amount = 8 }
   },
   outputs = {
      "atlanteans_soldier",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = _"recruiting soldier",
         actions = {
            "sleep=15000",
            "return=skipped unless economy needs atlanteans_soldier",
            "consume=tabard trident_light atlanteans_recruit",
            "animate=working 15000",
            "recruit=atlanteans_soldier"
         }
      },
   }
}
