dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_goldmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Deep Gold Mine"),
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
         template = "idle_??",
         directory = dirname,
         hotspot = { 49, 61 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 49, 61 },
         fps = 10
      },
      empty = {
         template = "empty_??",
         directory = dirname,
         hotspot = { 49, 61 },
      },
   },

   aihints = {
      mines = "gold"
   },

   working_positions = {
      empire_miner = 1,
      empire_miner_master = 1
   },

   inputs = {
      meal = 6,
      wine = 6
   },
   outputs = {
      "gold_ore"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _"mining gold",
         actions = {
            "sleep=43000",
            "return=skipped unless economy needs gold_ore",
            "consume=meal wine",
            "animate=working 18000",
            "mine=gold 2 100 5 2",
            "produce=gold_ore",
            "animate=working 18000",
            "mine=gold 2 100 5 2",
            "produce=gold_ore:2"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Main Gold Vein Exhausted",
      message =
         pgettext("empire_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}
