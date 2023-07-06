push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_stonemine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Stone Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 2,
   },
   return_on_dismantle = {
      log = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      empty = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}},
   },
   spritesheets = {
      working = {
         hotspot = {39, 46},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      }
   },

   aihints = {
      prohibited_till = 630
   },

   working_positions = {
      amazons_stonecutter = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _("mining granite"),
         actions = {
            "sleep=duration:5s",
            "return=skipped unless economy needs granite",
            "consume=ration",
            "sleep=duration:38s",
            "call=mine_produce",
            "call=mine_produce",
         }
      },
      mine_produce = {
         descname = _("mining granite"),
         actions = {
            "animate=working duration:10s",
            "mine=resource_stones radius:1 yield:100% when_empty:20%",
            "produce=granite",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=ration",
            "produce=granite:2",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Granite"),
      heading = _("Main Granite Vein Exhausted"),
      message =
         pgettext("amazons_building", "This stone mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}

pop_textdomain()
