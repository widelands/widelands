push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_mead_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Mead Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {50, 70}
      }
   },

   aihints = {},

   working_positions = {
      frisians_brewer = 1,
      frisians_brewer_master = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 },
      { name = "honey", amount = 6 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=brew_mead",
            "call=brew_beer",
            "call=brew_mead_2",
            "return=skipped"
         }
      },
      -- 2 identical programs for mead to prevent unnecessary skipping penalty
      brew_mead = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing mead because ...
         descname = _("brewing mead"),
         actions = {
            "return=skipped unless economy needs mead or workers need experience",
            "consume=barley water honey",
            "sleep=duration:30s",
            "animate=working duration:35s",
            "produce=mead"
         }
      },
      brew_mead_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing mead because ...
         descname = _("brewing mead"),
         actions = {
            "return=skipped unless economy needs mead or workers need experience",
            "consume=barley water honey",
            "sleep=duration:30s",
            "animate=working duration:35s",
            "produce=mead"
         }
      },
      brew_beer = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _("brewing beer"),
         actions = {
            "return=skipped unless economy needs beer",
            "consume=barley water",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=beer"
         }
      },
   },
}

pop_textdomain()
