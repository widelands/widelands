push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_hunter_gatherers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Hunter-Gatherer’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3,
      rope = 1
   },
   return_on_dismantle = {
      log = 1,
      rope = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}}
   },

   aihints = {
      needs_water = true,
      requires_supporters = true,
      --basic_amount = 1,
   },

   working_positions = {
      amazons_hunter_gatherer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _("gathering"),
         actions = {
            "call=hunt",
            "call=fish",
         }
      },
      hunt = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _("gathering meat"),
         actions = {
            -- time of worker: 8.2-62.2 sec, min+max average 35.2 sec
            -- min. time: 8.2 + 34 = 42.2 sec
            -- max. time: 62.2 + 34 = 96.2 sec
            -- avg. time: 35.2 + 34 = 69.2 sec
            "callworker=hunt",
            "sleep=duration:34s",
         }
      },
      fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _("gathering fish"),
         actions = {
            -- time of worker: 8.2-62.2 sec, min+max average 35.2 sec
            -- min. time: 8.2 + 34 = 42.2 sec
            -- max. time: 62.2 + 34 = 96.2 sec
            -- avg. time: 35.2 + 34 = 69.2 sec
            "callworker=fish",
            "sleep=duration:34s",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of Game and out of fish" for a resource
      title = _("No Game, No Fish"),
      -- TRANSLATORS: "Game" means animals that you can hunt
      heading = _("Out of Game and Fish"),
      -- TRANSLATORS: "game" means animals that you can hunt
      message = pgettext("amazons_building", "The hunter-gatherer working out of this hunter-gatherer’s hut can’t find any game or fish in her work area."),
      productivity_threshold = 33
   },
}

pop_textdomain()
