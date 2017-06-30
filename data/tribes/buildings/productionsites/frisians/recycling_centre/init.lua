dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_recycling_centre",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Recycling Centre"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 3,
      granite = 4,
      log = 5,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 2,
      log = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 86 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 86 },
      }
   },

   aihints = {
      prohibited_till = 10000,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      frisians_smelter = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "scrap_metal_iron", amount = 8 },
      { name = "scrap_metal_mixed", amount = 8 },
   },
   outputs = {
      "iron",
      "gold"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("frisians_building", "working"),
         actions = {
            "call=smelt_iron",
            "call=smelt_mixed",
            "return=skipped"
         }
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("frisians_building", "recycling iron"),
         actions = {
            "sleep=8000",
            "return=skipped unless economy needs iron",
            "consume=scrap_metal_iron:2 coal",
            "animate=working 22000",
            "produce=iron:2"
         }
      },
      smelt_mixed = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("frisians_building", "recycling iron and gold"),
         actions = {
            "sleep=8000",
            "return=skipped unless economy needs iron or economy needs gold",
            "consume=scrap_metal_mixed:2 coal",
            "animate=working 30000",
            "produce=iron gold"
         }
      },
   }
}
