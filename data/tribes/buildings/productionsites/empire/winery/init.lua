dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_winery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Winery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 1,
      granite = 1,
      marble = 2,
      marble_column = 1
   },
   return_on_dismantle = {
      granite = 1,
      marble = 1,
      marble_column = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 39, 62 },
      },
   },

   aihints = {
      prohibited_till = 560,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      empire_vintner = 1
   },

   inputs = {
      { name = "grape", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start making wine because ...
         descname = _"making wine",
         actions = {
            -- Grapes are only needed for wine, so no need to check if wine is needed
            "consume=grape:2",
            "sleep=duration:30s",
            "playsound=sound/empire/winebubble priority:40% allow_multiple",
            "animate=working duration:30s",
            "produce=wine"
         }
      },
   },
}
