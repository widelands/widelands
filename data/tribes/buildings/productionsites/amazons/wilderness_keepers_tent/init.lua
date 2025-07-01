push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_wilderness_keepers_tent",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Wilderness Keeper’s Tent"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      rope = 2
   },
   return_on_dismantle = {
      rope = 1
   },

   animation_directory = dirname,
   spritesheets = { idle = {
      hotspot = {34, 38},
      fps = 5,
      frames = 10,
      columns = 5,
      rows = 2
   }},

   aihints = {},

   working_positions = {
      amazons_wilderness_keeper = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- min. time total:  72.245 +  63.881 = 136.126 sec
            -- max. time total: 101.045 + 103.481 = 204.526 sec
            "call=breed",
            "call=release",
         },
      },
      breed = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("breeding fish"),
         actions = {
            -- time of worker: 19.745-48.545 sec
            -- min. time: 19.745 + 52.5 =  72.245 sec
            -- max. time: 48.545 + 52.5 = 101.045 sec
            "callworker=breed_fish",
            "sleep=duration:52s500ms",
         },
      },
      release = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("releasing game"),
         actions = {
            -- time of worker: 11.381-50.981 sec
            -- min. time: 11.381 + 52.5 =  63.881 sec
            -- max. time: 50.981 + 52.5 = 103.481 sec
            "callworker=release_game",
            "sleep=duration:52s500ms",
         },
      },
   },
}

pop_textdomain()
