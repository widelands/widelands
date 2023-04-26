push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_goldmine_deeper",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Deeper Gold Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 60, 37 },
      },
      empty = {
         hotspot = { 60, 37 },
      },
   },

   spritesheets = {
      working = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 60, 37 }
      },
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 60, 37 }
      },
   },

   aihints = {},

   working_positions = {
      barbarians_miner = 1,
      barbarians_miner_chief = 1,
      barbarians_miner_master = 1,
   },

   inputs = {
      { name = "meal", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _("mining gold"),
         actions = {
            -- "return=skipped" causes 10 sec delay
            -- time total: 27.6 + 4 * (9 + 3.6) + 10 = 88 sec
            "return=skipped unless economy needs gold_ore",
            "consume=meal",
            "sleep=duration:27s600ms",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "return=skipped"
         }
      },
      mine_produce = {
         descname = _("mining gold"),
         actions = {
            "animate=working duration:9s",
            "mine=resource_gold radius:2 yield:100% when_empty:10% experience_on_fail:2%",
            "produce=gold_ore",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=meal",
            "produce=gold_ore:4",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Gold"),
      heading = _("Main Gold Vein Exhausted"),
      message =
         pgettext("barbarians_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
