push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_mead_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Mead Brewery"),
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
            -- "return=skipped" causes 10 sec delay
            -- time total: 65.667 + 60.667 + 65.667 + 10 = 202 sec
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
            -- time: 27.067 + 35 + 3.6 = 65.667 sec
            "return=skipped unless economy needs mead or workers need experience",
            "consume=barley water honey",
            "sleep=duration:27s067ms",
            "animate=working duration:35s",
            "produce=mead"
         }
      },
      brew_mead_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing mead because ...
         descname = _("brewing mead"),
         actions = {
            -- time: 27.067 + 35 + 3.6 = 65.667 sec
            "return=skipped unless economy needs mead or workers need experience",
            "consume=barley water honey",
            "sleep=duration:27s067ms",
            "animate=working duration:35s",
            "produce=mead"
         }
      },
      brew_beer = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _("brewing beer"),
         actions = {
            -- time: 27.067 + 30 + 3.6 = 60.667 sec
            "return=skipped unless economy needs beer",
            "consume=barley water",
            "sleep=duration:27s067ms",
            "animate=working duration:30s",
            "produce=beer"
         }
      },
   },
}

pop_textdomain()
