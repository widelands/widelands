push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_rockmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Deep Rock Mine"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",

   spritesheets = {
      idle = {
         hotspot = {28, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {28, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      empty = {
         hotspot = {28, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {28, 57}
      }
   },

   aihints = {},

   working_positions = {
      frisians_miner = 1,
      frisians_miner_master = 1,
   },

   inputs = {
      { name = "meal", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _("mining granite"),
         actions = {
            -- "return=skipped" causes 10 sec delay
            -- time total: 5 + 24.9 + 7 * (8.7 + 3.6) + 10 = 126 sec
            "sleep=duration:5s",
            "return=skipped unless economy needs granite",
            "consume=meal",
            "sleep=duration:24s900ms",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "return=skipped"
         }
      },
      mine_produce = {
         descname = _("mining granite"),
         actions = {
            "animate=working duration:8s700ms",
            "mine=resource_stones radius:3 yield:100% when_empty:10% experience_on_fail:5%",
            "produce=granite",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=meal",
            "produce=granite:7",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Granite"),
      heading = _("Main Granite Vein Exhausted"),
      message =
         pgettext("frisians_building", "This rock mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
