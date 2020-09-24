push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Bakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 3
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 65 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 42, 65 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 43, 65 },
         fps = 2
      },
   },

   aihints = {
      prohibited_till = 550,
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_baker = 1
   },

   inputs = {
      { name = "water", amount = 6 },
      { name = "flour", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = pgettext("empire_building", "baking bread"),
         actions = {
            "return=skipped unless economy needs empire_bread",
            "consume=flour water",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "produce=empire_bread"
         }
      },
   },
}

pop_textdomain()
