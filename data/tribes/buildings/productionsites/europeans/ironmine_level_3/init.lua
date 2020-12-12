push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_ironmine_level_3",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Iron Mine Level 3"),
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "europeans_ironmine_level_4",
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
         hotspot = { 49, 49 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 49, 49 },
         fps = 10
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 49, 49 },
      },
   },

   aihints = {},

   working_positions = {
      europeans_miner_normal = 3
   },

   inputs = {
      { name = "snack", amount = 8 },
      { name = "mead", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining iron",
         actions = {
            "return=skipped unless economy needs ore or economy needs quartz",
            "consume=snack mead",
            "sleep=duration:20s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_quartz",
         }
      },
      mine_produce = {
         descname = _"mining iron",
         actions = {
            "animate=working duration:25s",
            "mine=resource_iron radius:8 yield:80% when_empty:20% experience_on_fail:10%",
            "produce=ore",
         }
      },
      mine_quartz = {
         descname = _"mining quartz",
         actions = {
            "animate=working duration:25s",
            "mine=resource_iron radius:8 yield:80% when_empty:20% experience_on_fail:10%",
            "produce=quartz",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=snack mead",
            "produce=ore:4 quartz",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Iron",
      heading = _"Main Iron Vein Exhausted",
      message =
         pgettext("europeans_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
