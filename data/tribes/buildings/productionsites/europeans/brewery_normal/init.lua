push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_brewery_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",
   
   enhancement = {
      name = "europeans_brewery_advanced",
      enhancement_cost = {
          marble_column = 2,
          quartz = 1,
          diamond = 1
      },
      enhancement_return_on_dismantle = {
          marble = 2,
          quartz = 1,
          diamond = 1
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 60, 59 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 60, 59 },
      },
   },

   aihints = {
      prohibited_till = 1800
   },

   working_positions = {
      europeans_brewer_normal = 1
   },

   inputs = {
      { name = "water", amount = 12 },
      { name = "barley", amount = 10 },
      { name = "honey", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=brew_beer_basic",
            "call=brew_beer",
            "call=brew_mead",
            "call=brew_strong_beer"
         }
      },
      brew_beer_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing beer",
         actions = {
            "return=skipped when economy needs beer",
            "return=skipped when economy needs mead",
            "return=skipped when economy needs strong_beer",
            "consume=water barley",
            "sleep=duration:60s",
            "animate=working duration:60s",
            "produce=beer"
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
            "sleep=duration:60s",
            "animate=working duration:60s",
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
            "sleep=duration:60s",
            "animate=working duration:60s",
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
            "sleep=duration:90s",
            "animate=working duration:90s",
            "produce=beer_strong:3"
         }
      },
   },
}

pop_textdomain()
