push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_rare_tree_cutters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Rare Tree Cutter’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {40, 66}},
      unoccupied = {hotspot = {39, 66}}
   },

   aihints = {
      basic_amount = 1,
      requires_supporters = true,
   },

   working_positions = {
      amazons_woodcutter_master = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- if all are needed, then time for one of each is the same as for harvest_all
            -- if only one is needed then total time is (2 on-demand and one of each unconditional):
            --    min.: 5 * 47.2 = 236 sec (not actually possible to have all 5 at 1 step from flag)
            --    max.: 5 * 86.8 = 434 sec
            --    avg.: 5 * 67 = 335 sec
            -- average times for the needed rare tree:
            --    min.: 236 / 3 = 78.7 sec
            --    max.: 434 / 3 = 144.7 sec
            --    avg.: 335 / 3 = 111.7 sec
            "call=harvest_demand",
            "call=harvest_all",
            "call=harvest_demand",
         },
      },
      harvest_demand = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- min. time if only one is needed: 47.2 sec
            -- max. time if only one is needed: 86.8 sec
            -- avg. time if only one is needed: 67 sec
            -- if all are needed, then time is the same as for harvest_all
            "call=harvest_balsa_if_needed",
            "call=harvest_rubber_if_needed",
            "call=harvest_ironwood_if_needed",
         },
      },
      harvest_all = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- min. time total: 3 * 47.2 = 141.6 sec
            -- max. time total: 3 * 86.8 = 260.4 sec
            -- avg. time total: 3 * 67 = 201 sec
            "call=harvest_balsa",
            "call=harvest_rubber",
            "call=harvest_ironwood",
         },
      },
      harvest_balsa_if_needed = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting balsa because ...
         descname = _("harvesting balsa"),
         actions = {
            "return=skipped unless economy needs balsa",
            "call=harvest_balsa",
         },
      },
      harvest_rubber_if_needed = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting rubber because ...
         descname = _("harvesting rubber"),
         actions = {
            "return=skipped unless economy needs rubber",
            "call=harvest_rubber",
         },
      },
      harvest_ironwood_if_needed = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting ironwood because ...
         descname = _("harvesting ironwood"),
         actions = {
            "return=skipped unless economy needs ironwood",
            "call=harvest_ironwood",
         },
      },
      harvest_balsa = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting balsa because ...
         descname = _("harvesting balsa"),
         actions = {
            -- time of worker: 35.2-74.8, min+max average 55 sec
            -- min. time: 35.2 + 12 = 47.2 sec
            -- max. time: 74.8 + 12 = 86.8 sec
            -- avg. time: 55 + 12 = 67 sec
            "callworker=harvest_balsa",
            "sleep=duration:12s"
         },
      },
      harvest_rubber = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting rubber because ...
         descname = _("harvesting rubber"),
         actions = {
            -- time of worker: 35.2-74.8, min+max average 55 sec
            -- min. time: 35.2 + 12 = 47.2 sec
            -- max. time: 74.8 + 12 = 86.8 sec
            -- avg. time: 55 + 12 = 67 sec
            "callworker=harvest_rubber",
            "sleep=duration:12s"
         },
      },
      harvest_ironwood = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting ironwood because ...
         descname = _("harvesting ironwood"),
         actions = {
            -- time of worker: 35.2-74.8, min+max average 55 sec
            -- min. time: 35.2 + 12 = 47.2 sec
            -- max. time: 74.8 + 12 = 86.8 sec
            -- avg. time: 55 + 12 = 67 sec
            "callworker=harvest_ironwood",
            "sleep=duration:12s"
         },
      },
   },
}

pop_textdomain()
