push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_coalmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Deep Coal Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "barbarians_coalmine_deeper",
      enhancement_cost = {
         log = 4,
         granite = 2
      },
      enhancement_return_on_dismantle = {
         log = 2,
         granite = 1
      }
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
      mines = "resource_coal",
      mines_percent = 60
   },

   working_positions = {
      barbarians_miner = 1,
      barbarians_miner_chief = 1,
   },

   inputs = {
      { name = "snack", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "return=skipped unless economy needs coal",
            "consume=snack",
            "sleep=duration:40s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
         }
      },
      mine_produce = {
         descname = _"mining coal",
         actions = {
            "animate=working duration:9s500ms",
            "mine=resource_coal radius:2 yield:66.66% when_empty:5% experience_on_fail:17%",
            "produce=coal",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=snack",
            "produce=coal:4",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Coal",
      heading = _"Main Coal Vein Exhausted",
      message =
         pgettext("barbarians_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
