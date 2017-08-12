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
      brick = 4,
      granite = 4,
      log = 3,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 86 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), --TODO no animation yet
         hotspot = { 57, 86 },
      }
   },

   aihints = {
      prohibited_till = 1200,
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
      { name = "fur_clothes_old", amount = 8 },
   },
   outputs = {
      "iron",
      "gold",
      "fur"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = pgettext("frisians_building", "working"),
         actions = {
            "call=smelt_iron",
            "call=smelt_mixed",
            "call=make_fur",
            "return=skipped"
         }
      },
      make_fur = {
         -- TRANSLATORS: Completed/Skipped/Did not start recycling fur because ...
         descname = pgettext("frisians_building", "recycling fur"),
         actions = {
            "return=skipped unless site has fur_clothes_old",
            "sleep=14000",
            "consume=fur_clothes_old",
            "animate=working 15000",
            "produce=fur"
         }
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start recycling iron because ...
         descname = pgettext("frisians_building", "recycling iron"),
         actions = {
            "return=skipped unless economy needs iron",
            "sleep=14000",
            "consume=scrap_metal_iron:2 coal",
            "animate=working 18000",
            "produce=iron:2"
         }
      },
      smelt_mixed = {
         -- TRANSLATORS: Completed/Skipped/Did not start recycling iron and gold because ...
         descname = pgettext("frisians_building", "recycling iron and gold"),
         actions = {
            "return=skipped unless economy needs iron or economy needs gold",
            "sleep=14000",
            "consume=scrap_metal_mixed:2 coal",
            "animate=working 24000",
            "produce=iron gold"
         }
      },
   }
}
