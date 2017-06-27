dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_tannery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Tannery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      log = 6,
      thatch_reed = 4
   },
   return_on_dismantle = {
      brick = 2,
      log = 3,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 88 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 88 },
      },
   },

   aihints = {
      forced_after = 900
   },

   working_positions = {
      frisians_tanner = 1
   },

   inputs = {
      { name = "meat", amount = 4 },
      { name = "fish", amount = 4 },
      { name = "log", amount = 6 },
   },
   outputs = {
      "smoked_meat",
      "smoked_fish",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"working",
         actions = {
            "call=smoke_meat",
            "call=smoke_fish",
            "return=skipped"
         },
      },
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"smoking fish",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs smoked_fish",
            "consume=fish log",
            "animate=working 19000",
            "produce=smoked_fish"
         },
      },
      smoke_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"smoking meat",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs smoked_meat",
            "consume=meat log",
            "animate=working 19000",
            "produce=smoked_meat"
         },
      },
   },
}
