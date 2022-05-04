push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_goldmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Deep Gold Mine"),
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
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _("mining gold"),
         actions = {
            "return=skipped unless economy needs gold_ore",
            "consume=meal wine",
            "sleep=duration:40s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "return=skipped"
         }
      },
      mine_produce = {
         descname = _("mining gold"),
         actions = {
            "animate=working duration:13s",
            "mine=resource_gold radius:2 yield:100% when_empty:5% experience_on_fail:2%",
            "produce=gold_ore",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=meal wine",
            "produce=gold_ore:3",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Gold"),
      heading = _("Main Gold Vein Exhausted"),
      message =
         pgettext("empire_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
