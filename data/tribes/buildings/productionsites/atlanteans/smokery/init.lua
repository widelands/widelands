dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_smokery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Smokery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      granite = 4,
      planks = 1,
      spidercloth = 1
   },
   return_on_dismantle = {
      granite = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 53, 68 },
         fps = 20
      }
   },

   aihints = {
      is_basic = true,
      prohibited_till = 180,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_smoker = 1
   },

   inputs = {
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "log", amount = 8 }
   },
   outputs = {
      "smoked_meat",
      "smoked_fish"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smoke_fish",
            "call=smoke_meat",
            "call=smoke_fish",
            "return=skipped"
         }
      },
      smoke_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            -- time total: 60
            "return=skipped when site has fish and economy needs smoked_fish and not economy needs smoked_meat",
            "return=skipped unless economy needs smoked_meat",
            "sleep=10000",
            "consume=meat:2 log",
            "animate=working 30000",
            "sleep=20000",
            "produce=smoked_meat:2"
         }
      },
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            -- time total: 60
            "return=skipped when site has meat and economy needs smoked_meat and not economy needs smoked_fish",
            "return=skipped unless economy needs smoked_fish",
            "sleep=10000",
            "consume=fish:2 log",
            "animate=working 30000",
            "sleep=20000",
            "produce=smoked_fish:2"
         }
      },
   },
}
