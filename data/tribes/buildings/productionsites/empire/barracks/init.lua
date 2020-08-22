dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Barracks"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      granite = 3,
      planks = 2,
      marble = 2,
      marble_column = 2,
      cloth = 1
   },
   return_on_dismantle = {
      granite = 2,
      planks = 1,
      marble = 1
   },

   animations = {
      idle = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 56, 68 },
      },
      working = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 56, 68 },
      }
   },

   aihints = {
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      prohibited_till = 920
   },

   working_positions = {
      empire_trainer = 1
   },

   inputs = {
      { name = "armor_helmet", amount = 8 },
      { name = "spear_wooden", amount = 8 },
      { name = "empire_recruit", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("empire_building", "recruiting soldier"),
         actions = {
            "return=skipped unless economy needs empire_soldier",
            "consume=armor_helmet spear_wooden empire_recruit",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "recruit=empire_soldier"
         }
      },
   }
}
