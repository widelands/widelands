push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
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
         descname = _"working",
         actions = {
            "call=breed",
            "call=release",
         },
      },
      breed = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"breeding fish",
         actions = {
            "callworker=breed_fish",
            "sleep=duration:52s500ms",
         },
      },
      release = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"releasing game",
         actions = {
            "callworker=release_game",
            "sleep=duration:52s500ms",
         },
      },
   },
}

pop_textdomain()
