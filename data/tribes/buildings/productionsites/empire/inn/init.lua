push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Inn"),
   icon = dirname .. "menu.png",
   size = "medium",

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 46, 74 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 46, 74 },
      },
   },

   aihints = {
      prohibited_till = 880
   },

   working_positions = {
      empire_innkeeper = 1
   },

   inputs = {
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 },
      { name = "empire_bread", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_ration",
            "call=produce_meal",
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _("preparing a ration"),
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=empire_bread,fish,meat",
            "playsound=sound/empire/taverns/ration priority:80%",
            "animate=working duration:23s",
            "sleep=duration:10s",
            "produce=ration"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _("preparing a meal"),
         actions = {
            -- time total: 40
            "return=skipped unless economy needs meal",
            "consume=empire_bread fish,meat",
            "playsound=sound/empire/taverns/meal priority:80%",
            "animate=working duration:30s",
            "sleep=duration:10s",
            "produce=meal"
         }
      },
   },
}

pop_textdomain()
