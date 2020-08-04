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
      basic_amount = 1,
      prohibited_till = 580,
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

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smoke_fish",
            "call=smoke_meat",
            "call=smoke_fish_2",
         }
      },
      smoke_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            -- time total: 60
            "return=skipped unless economy needs smoked_meat",
            "consume=meat:2 log",
            "animate=working duration:30s",
            "sleep=30000",
            "produce=smoked_meat:2"
         }
      },
      -- 2 identical programs for fish to prevent unnecessary skipping penalty
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            -- time total: 60
            "return=skipped unless economy needs smoked_fish",
            "consume=fish:2 log",
            "animate=working duration:30s",
            "sleep=30000",
            "produce=smoked_fish:2"
         }
      },
      smoke_fish_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            -- time total: 60
            "return=skipped unless economy needs smoked_fish",
            "consume=fish:2 log",
            "animate=working duration:30s",
            "sleep=30000",
            "produce=smoked_fish:2"
         }
      },
   },
}
