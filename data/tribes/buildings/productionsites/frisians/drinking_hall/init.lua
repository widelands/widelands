push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_drinking_hall",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Drinking Hall"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   spritesheets = {
      idle = {
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {50, 72}
      }
   },

   aihints = {
      prohibited_till = 930
   },

   working_positions = {
      frisians_landlady = 1
   },

   inputs = {
      { name = "fruit", amount = 4 },
      { name = "honey_bread", amount = 4 },
      { name = "beer", amount = 4 },
      { name = "smoked_fish", amount = 4 },
      { name = "smoked_meat", amount = 4 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- time total: 37 + 44 = 81 sec
            "call=produce_ration",
            "call=produce_meal",
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _("preparing a ration"),
         actions = {
            -- time: 5.4 + 18 + 10 + 3.6 = 37 sec
            "return=skipped unless economy needs ration",
            "consume=fruit,smoked_fish,smoked_meat",
            "sleep=duration:5s400ms",
            "animate=working duration:18s",
            "sleep=duration:10s",
            "produce=ration"
         }
      },
      produce_meal = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a meal because ...
         descname = _("preparing a meal"),
         actions = {
            -- time: 30.4 + 10 + 3.6 = 44 sec
            "return=skipped unless economy needs meal",
            "consume=beer honey_bread smoked_fish,smoked_meat",
            "animate=working duration:30s400ms",
            "sleep=duration:10s",
            "produce=meal"
         }
      },
   },
}

pop_textdomain()
