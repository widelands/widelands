dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_smokery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Smokery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 68 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 53, 78 },
         fps = 20
      }
   },

   aihints = {
      prohibited_till = 300,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      frisians_smoker = 1
   },

   inputs = {
      { name = "meat", amount = 6 },
      { name = "fish", amount = 6 },
      { name = "log", amount = 6 },
   },
   outputs = {
      "smoked_meat",
      "smoked_fish",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smoke_meat",
            "call=smoke_fish",
            "return=skipped"
         },
      },
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            "return=skipped unless economy needs smoked_fish",
            "sleep=12000",
            "consume=fish:2 log",
            "animate=working 19000",
            "produce=smoked_fish:2"
         },
      },
      smoke_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            "return=skipped when site has fish:2 and economy needs smoked_fish",
            "return=skipped unless economy needs smoked_meat",
            "sleep=12000",
            "consume=meat:2 log",
            "animate=working 19000",
            "produce=smoked_meat:2"
         },
      },
   },
}
