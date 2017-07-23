dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_mead_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Mead Brewery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 3,
      granite = 2,
      log = 1,
      thatch_reed = 3
   },
   return_on_dismantle_on_enhanced = {
      brick = 3,
      granite = 1,
      log = 1,
      thatch_reed = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 56, 47 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 56, 47 },
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      frisians_brewer = 1,
      frisians_brewer_master = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 },
      { name = "honey", amount = 6 },
   },
   outputs = {
      "mead",
      "beer"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=brew_mead",
            "call=brew_beer",
            "return=skipped"
         }
      },
      brew_mead = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing mead because ...
         descname = _"brewing mead",
         actions = {
            "return=skipped unless economy needs mead or workers need experience",
            "sleep=10000",
            "consume=barley water honey",
            "animate=working 44000",
            "produce=mead"
         }
      },
      brew_beer = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing mead because ...
         descname = _"brewing beer",
         actions = {
            "return=skipped unless economy needs beer or workers need experience",
            "sleep=10000",
            "consume=barley water",
            "animate=working 35000",
            "produce=beer"
         }
      },
   },
}
