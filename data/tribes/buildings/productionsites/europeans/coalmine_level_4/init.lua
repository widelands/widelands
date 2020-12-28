push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_coalmine_level_4",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Coal Mine Level 4"),
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "europeans_coalmine_level_5",
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

   aihints = {},

   working_positions = {
      europeans_miner_normal = 2,
      europeans_miner_advanced = 1
   },

   inputs = {
      { name = "snack", amount = 6 },
      { name = "mead", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "return=skipped unless economy needs coal or economy needs diamond",
            "consume=snack mead",
            "sleep=duration:15s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_diamond",
         }
      },
      mine_produce = {
         descname = _"mining coal",
         actions = {
            "animate=working duration:30s",
            "mine=resource_coal radius:8 yield:90% when_empty:20% experience_on_fail:10%",
            "produce=coal",
         }
      },
      mine_diamond = {
         descname = _"mining diamond",
         actions = {
            "animate=working duration:30s",
            "mine=resource_coal radius:8 yield:90% when_empty:20% experience_on_fail:10%",
            "produce=diamond",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=snack mead",
            "produce=coal:4 diamond",
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
