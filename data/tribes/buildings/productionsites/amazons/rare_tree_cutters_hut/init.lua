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
            "call=harvest_balsa",
            "call=harvest_rubber",
            "call=harvest_ironwood",
         },
      },
      harvest_balsa = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _("harvesting balsa"),
         actions = {
            -- time total: xx
            "callworker=harvest_balsa",
            "sleep=duration:12s"
         },
      },
      harvest_rubber = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _("harvesting rubber"),
         actions = {
            -- time total: xx
            "callworker=harvest_rubber",
            "sleep=duration:12s"
         },
      },
      harvest_ironwood = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _("harvesting ironwood"),
         actions = {
            -- time total: xx
            "callworker=harvest_ironwood",
            "sleep=duration:12s"
         },
      },
   },
}

pop_textdomain()
