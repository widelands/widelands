dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_marblemine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Deep Marble Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement_cost = {
      log = 4,
      planks = 2
   },
   return_on_dismantle_on_enhanced = {
      log = 2,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 49, 61 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 49, 61 },
         fps = 10
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 49, 61 },
      },
   },

   aihints = {
      mines = "stones"
   },

   working_positions = {
      empire_miner = 1,
      empire_miner_master = 1
   },

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "meal", amount = 6 },
      { name = "wine", amount = 6 }
   },
   outputs = {
      "marble",
      "granite"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining marble because ...
         descname = _"mining marble",
         actions = {
            "sleep=43000",
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=meal wine",
            "animate=working 18000",
            "mine=stones 2 100 5 2",
            "produce=marble:2",
            "animate=working 18000",
            "mine=stones 2 100 5 2",
            "produce=marble granite:2"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Marble",
      heading = _"Main Marble Vein Exhausted",
      message =
         pgettext("empire_building", "This marble mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}
