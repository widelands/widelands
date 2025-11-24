push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_ironmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Deep Iron Mine"),
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
         hotspot = {28, 57},
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
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _("mining iron"),
         actions = {
            -- "return=skipped" causes 10 sec delay
            -- time total: 50.9 + 7 * (8.7 + 3.6) + 10 = 147 sec
            "return=skipped unless economy needs iron_ore",
            "consume=meal",
            "sleep=duration:50s900ms",
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
         descname = _("mining iron"),
         actions = {
            "animate=working duration:8s700ms",
            "mine=resource_iron radius:3 yield:100% when_empty:10% experience_on_fail:5%",
            "produce=iron_ore",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=meal",
            "produce=iron_ore:7",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Iron"),
      heading = _("Main Iron Vein Exhausted"),
      message =
         pgettext("frisians_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
