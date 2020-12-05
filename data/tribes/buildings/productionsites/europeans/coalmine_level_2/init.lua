push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_coalmine_level_2",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Coal Mine Level 2"),
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "europeans_coalmine_level_3",
      enhancement_cost = {
      log = 2,
      planks = 1,
      reed = 1
      
      },
      enhancement_return_on_dismantle = {
         log = 1
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 60, 37 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 60, 37 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 60, 37 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 60, 37 },
      },
   },

   aihints = {},

   working_positions = {
      europeans_miner_basic = 2,
      europeans_miner_normal = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 4 },
      { name = "smoked_meat", amount = 4 },
      { name = "europeans_bread", amount = 8 },
      { name = "beer", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "return=skipped unless economy needs coal",
            "consume=smoked_fish,smoked_meat europeans_bread beer",
            "sleep=duration:25s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
         }
      },
      mine_produce = {
         descname = _"mining coal",
         actions = {
            "animate=working duration:20s",
            "mine=resource_coal radius:4 yield:60% when_empty:15% experience_on_fail:10%",
            "produce=coal",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=smoked_fish,smoked_meat europeans_bread beer",
            "produce=coal:4",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Coal",
      heading = _"Main Coal Vein Exhausted",
      message =
         pgettext("europeans_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
