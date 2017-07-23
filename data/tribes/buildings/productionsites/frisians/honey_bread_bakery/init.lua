dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_honey_bread_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Honey Bread Bakery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 3,
      log = 3,
      granite = 2,
      thatch_reed = 4
   },
   return_on_dismantle_on_enhanced = {
      brick = 3,
      granite = 2,
      log = 2,
      thatch_reed = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 51, 84 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 51, 84 },
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      frisians_baker = 1,
      frisians_baker_master = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 },
      { name = "honey", amount = 6 },
   },
   outputs = {
      "honey_bread",
      "bread_frisians"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=bake_honey",
            "call=bake_normal",
            "return=skipped"
         }
      },
      bake_honey = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking honey bread because ...
         descname = _"baking honey bread",
         actions = {
            "return=skipped unless economy needs honey_bread or workers need experience",
            "sleep=10000",
            "consume=barley water honey",
            "animate=working 51000",
            "produce=honey_bread"
         }
      },
      bake_normal = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _"baking bread",
         actions = {
            "return=skipped unless economy needs bread_frisians or workers need experience",
            "sleep=10000",
            "consume=barley water",
            "animate=working 40000",
            "produce=bread_frisians"
         }
      },
   },
}
