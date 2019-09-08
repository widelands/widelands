dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_goldmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Deep Gold Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "barbarians_goldmine_deeper",

   enhancement_cost = {
      log = 4,
      granite = 2
   },
   return_on_dismantle_on_enhanced = {
      log = 2,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 21, 37 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 21, 37 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 21, 37 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 21, 37 },
      },
   },

   aihints = {
      mines = "gold",
      mines_percent = 60
   },

   working_positions = {
      barbarians_miner = 1,
      barbarians_miner_chief = 1,
   },

   inputs = {
      { name = "snack", amount = 6 }
   },
   outputs = {
      "gold_ore"
   },

   indicate_workarea_overlaps = {
      barbarians_goldmine = false,
      barbarians_goldmine_deep = false,
      barbarians_goldmine_deeper = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _"mining gold",
         actions = {
            "return=skipped unless economy needs gold_ore",
            "consume=snack",
            "sleep=40000",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "return=no_stats"
         }
      },
      mine_produce = {
         descname = _"mining gold",
         actions = {
            "animate=working 9500",
            "mine=gold 2 66 5 17",
            "produce=gold_ore",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=snack",
            "produce=gold_ore:4",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Gold",
      heading = _"Main Gold Vein Exhausted",
      message =
         pgettext("barbarians_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
