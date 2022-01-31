push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "amazons_food_preserver",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Food Preserver"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 3,
      rope = 2,
      rubber =1
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      rope = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 44}},
      unoccupied = {hotspot = {43, 44}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 44},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
      prohibited_till = 520,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      --basic_amount = 1
   },

   working_positions = {
      amazons_cook = 1
   },

   inputs = {
      { name = "coal", amount = 3 },
      { name = "amazons_bread", amount = 6 },
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _("preparing a ration"),
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=amazons_bread:2 fish,meat:2 coal",
            "sleep=duration:5s",
            "animate=working duration:18s",
            "sleep=duration:10s",
            "produce=ration:2"
         },
      },
   },
}

pop_textdomain()
