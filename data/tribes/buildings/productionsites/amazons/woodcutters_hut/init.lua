push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_woodcutters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Woodcutter’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   enhancement = {
      name = "amazons_rare_tree_cutters_hut",
      enhancement_cost = {
         log = 1,
         granite = 1,
         rope = 1
      },
      enhancement_return_on_dismantle = {
         log = 1,
         rope = 1
      }
   },

   buildcost = {
      log = 4,
   },
   return_on_dismantle = {
      log = 2,
   },

   animation_directory = dirname,
   animations = {
      idle = { hotspot = {41, 46}},
      unoccupied = { hotspot = {39, 46}}
   },

   aihints = {},

   working_positions = {
      amazons_woodcutter = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _("felling trees"),
         actions = {
            "return=skipped unless economy needs log",
            "callworker=harvest",
            "sleep=duration:10s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Trees"),
      heading = _("Out of Trees"),
      message = pgettext("amazons_building", "The woodcutter working at this woodcutter’s hut can’t find any trees in her work area. You should consider dismantling or destroying the building or building a jungle preserver’s hut."),
      productivity_threshold = 66
   },
}

pop_textdomain()
