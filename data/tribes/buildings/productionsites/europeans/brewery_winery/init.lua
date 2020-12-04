push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_brewery_winery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Brewery and Winery"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 39, 62 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_brewer_advanced = 2
   },

   inputs = {
      { name = "water", amount = 12 },
      { name = "barley", amount = 10 },
      { name = "honey", amount = 6 },
      { name = "fruit", amount = 8 },
      { name = "grape", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=brew_beer",
            "call=brew_mead",
            "call=brew_strong_beer",
            "call=making_wine"
         }
      },
      brew_beer = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing beer",
         actions = {
            "return=skipped unless economy needs beer",
            "return=skipped when economy needs water",
            "return=skipped when site has honey and economy needs mead and not economy needs beer",
            "return=skipped when site has barley and economy needs beer_strong and not economy needs beer",
            "consume=water:3 barley:3",
            "sleep=duration:50s",
            "animate=working duration:50s",
            "produce=beer:3"
         }
      },
      brew_mead = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing mead",
         actions = {
            "return=skipped unless economy needs mead",
            "return=skipped when economy needs water",
            "return=skipped when site has barley and economy needs beer and not economy needs mead",
            "return=skipped when site has barley and economy needs beer_strong and not economy needs mead",
            "consume=water:3 barley:3 honey:3",
            "sleep=duration:50s",
            "animate=working duration:50s",
            "produce=mead:3"
         }
      },
      brew_strong_beer = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing strong beer because ...
         descname = _"brewing strong beer",
         actions = {
            "return=skipped unless economy needs beer_strong",
            "return=skipped when economy needs water",
            "return=skipped when site has barley and economy needs beer and not economy needs beer_strong",
            "return=skipped when site has honey and economy needs mead and not economy needs beer_strong",
            "consume=water:4 barley:4",
            "sleep=duration:80s",
            "animate=working duration:80s",
            "produce=beer_strong:3"
         }
      },
      making_wine = {
         -- TRANSLATORS: Completed/Skipped/Did not start making wine because ...
         descname = _"making wine",
         actions = {
            -- Grapes are only needed for wine, so no need to check if wine is needed
            "return=skipped unless economy needs wine",
            "consume=water fruit grape:4",
            "sleep=duration:60s",
            "playsound=sound/empire/winebubble priority:40% allow_multiple",
            "animate=working duration:60s",
            "produce=wine:3"
         }
      },
   },
}

pop_textdomain()
