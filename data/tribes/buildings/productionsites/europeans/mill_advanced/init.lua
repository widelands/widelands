push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_mill_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Mill"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 87 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 41, 87 },
         fps = 25
      },
   },

   aihints = {
      prohibited_till = 7200
   },

   working_positions = {
      europeans_miller_advanced = 1
   },

   inputs = {
      { name = "corn", amount = 6 },
      { name = "rye", amount = 4 },
      { name = "wheat", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_cornmeal",
            "call=produce_mixed_flour",
         }
      },
      produce_cornmeal = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding corn because ...
         descname = _"grinding corn",
         actions = {
            "return=skipped when site has rye and economy needs flour and not economy needs cornmeal",
            "return=skipped unless economy needs cornmeal",
            "sleep=duration:10s",
            "consume=corn:2",
            "playsound=sound/mill/mill_turning priority:85% allow_multiple",
            "animate=working duration:20s",
            "produce=cornmeal:2"
         }
      },
      produce_mixed_flour = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding blackroot because ...
         descname = _"grinding rye and wheat",
         actions = {
            -- No check whether we need blackroot_flour because blackroots cannot be used for anything else.
            "return=skipped when site has corn and economy needs cornmeal and not economy needs flour",
            "consume=wheat:2 rye:2",
            "sleep=duration:10s",
            "playsound=sound/mill/mill_turning priority:85% allow_multiple",
            "animate=working duration:20s",
            "produce=flour:3"
         }
      },
   },
}

pop_textdomain()
