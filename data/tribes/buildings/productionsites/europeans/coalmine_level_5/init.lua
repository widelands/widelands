push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_coalmine_level_5",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Coal Mine Level 5"),
   icon = dirname .. "menu.png",
   size = "mine",


   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 56 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 50, 56 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 50, 56 },
      },
   },

   aihints = {},

   working_positions = {
      europeans_miner_advanced = 3
   },

   inputs = {
      { name = "smoked_fish", amount = 4 },
      { name = "smoked_meat", amount = 4 },
      { name = "europeans_bread", amount = 8 },
      { name = "wine", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining coal",
         actions = {
            "return=skipped unless economy needs coal or economy needs diamond",
            "consume=smoked_fish,smoked_meat europeans_bread wine",
            "sleep=duration:10s",
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
            "animate=working duration:35s",
            "mine=resource_coal radius:12 yield:100% when_empty:30% experience_on_fail:10%",
            "produce=coal",
         }
      },
      mine_diamond = {
         descname = _"mining diamond",
         actions = {
            "animate=working duration:35s",
            "mine=resource_coal radius:12 yield:100% when_empty:30% experience_on_fail:10%",
            "produce=diamond",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=smoked_fish,smoked_meat europeans_bread wine",
            "produce=coal:4 diamond",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Coal",
      heading = _"Main Coal Vein Exhausted",
      message =
         pgettext("europeans_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}

pop_textdomain()
