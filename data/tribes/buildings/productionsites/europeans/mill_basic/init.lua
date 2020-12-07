push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_mill_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Mill"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_mill_normal",
        enhancement_cost = {
          planks = 2,
          brick = 1,
          grout = 1,
          spidercloth = 2,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 1,
          quartz = 1,
        },
   },

   buildcost = {
      reed = 2,
      planks = 2,
      brick = 3,
      grout = 3
   },
   return_on_dismantle = {
      log = 4,
      granite = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 88 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 57, 88 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 88 },
      },
   },

   aihints = {
      prohibited_till = 1800
   },

   working_positions = {
      europeans_miller_basic = 1
   },

   inputs = {
      { name = "corn", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_cornmeal"
         }
      },
      produce_cornmeal = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding corn because ...
         descname = _"grinding corn",
         actions = {
            "return=skipped unless economy needs cornmeal",
            "sleep=duration:45s",
            "consume=corn:2",
            "playsound=sound/mill/mill_turning priority:85% allow_multiple",
            "animate=working duration:45s",
            "produce=cornmeal:2"
         }
      }
   },
}

pop_textdomain()
