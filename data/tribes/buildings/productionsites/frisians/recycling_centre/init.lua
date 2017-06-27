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
      brick = 2,
      granite = 3,
      log = 5,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      granite = 2,
      log = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 64 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 64 },
      }
   },

   aihints = {
      forced_after = 1000,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      frisians_smelter = 1
   },

   inputs = {
      { name = "scrap_metal", amount = 8 },
      { name = "coal", amount = 8 },
   },
   outputs = {
      "iron",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("frisians_building", "recycling metal"),
         actions = {
            "sleep=32000",
            "return=skipped unless economy needs iron",
            "consume=scrap_metal:3 coal",
            "animate=working 15000",
            "produce=iron:2"
         }
      },
   }
}
