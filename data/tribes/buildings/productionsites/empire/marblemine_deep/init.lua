push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_marblemine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Deep Marble Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   animation_directory = dirname,

   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 51, 66 },
      },
      empty = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 51, 66 },
      },
      working = {
         fps = 10,
         frames = 10,
         columns = 10,
         rows = 1,
         hotspot = { 51, 66 },
      },
   },

   aihints = {},

   working_positions = {
      empire_miner = 1,
      empire_miner_master = 1
   },

   inputs = {
      { name = "meal", amount = 6 },
      { name = "wine", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining marble because ...
         descname = _("mining marble"),
         actions = {
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=meal wine",
            "sleep=duration:40s",
            "call=mine_produce_marble",
            "call=mine_produce_granite",
            "call=mine_produce_marble",
            "call=mine_produce_granite",
            "call=mine_produce_marble",
            "return=skipped"
         }
      },
      mine_produce_granite = {
         descname = _("mining marble"),
         actions = {
            "animate=working duration:7s800ms",
            "mine=resource_stones radius:2 yield:100% when_empty:5% experience_on_fail:2%",
            "produce=granite",
         }
      },
      mine_produce_marble = {
         descname = _("mining marble"),
         actions = {
            "animate=working duration:7s800ms",
            "mine=resource_stones radius:2 yield:100% when_empty:5% experience_on_fail:2%",
            "produce=marble",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=meal wine",
            "produce=marble:3 granite:2",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Marble"),
      heading = _("Main Marble Vein Exhausted"),
      message =
         pgettext("empire_building", "This marble mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
