dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_coalmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Deep Coal Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement_cost = {
      brick = 2,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle_on_enhanced = {
      brick = 2,
      granite = 1,
      log = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 60, 37 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 60, 37 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 60, 37 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 60, 37 },
      }
   },

   aihints = {
      mines = "coal",
   },

   working_positions = {
      frisians_miner = 1,
      frisians_miner_master = 1,
   },

   inputs = {
      { name = "meal", amount = 8 }
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "sleep=20000",
            "return=skipped unless economy needs coal",
            "consume=meal",
            "animate=working 10000",
            "mine=coal 3 100 10 5",
            "produce=coal",
            "animate=working 10000",
            "mine=coal 3 100 10 5",
            "produce=coal",
            "animate=working 10000",
            "mine=coal 3 100 10 5",
            "produce=coal"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Coal",
      heading = _"Main Coal Vein Exhausted",
      message =
         pgettext("frisians_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}
