push_textdomain("tribes")

local dirname = path.dirname (__file__)

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
            -- time of woodcutter: 39.2-78.8 sec, min+max average 59 sec
            -- min. time total: 39.2 + 10 = 49.2 sec
            -- max. time total: 78.8 + 10 = 88.8 sec
            -- avg. time total: 59 + 10 = 69 sec
            -- time of master woodcutter: 34.2-73.8, min+max average 54 sec
            -- min. time total: 34.2 + 10 = 44.2 sec
            -- max. time total: 73.8 + 10 = 83.8 sec
            -- avg. time total: 54 + 10 = 64 sec
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
