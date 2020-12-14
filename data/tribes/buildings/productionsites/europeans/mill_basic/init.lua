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
          brick = 2,
          grout = 2,
          spidercloth = 2,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 2,
          quartz = 1,
        },
   },

   buildcost = {
      reed = 3,
      planks = 3,
      granite = 1
   },
   return_on_dismantle = {
      log = 3,
      granite = 1
   },

   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = { 52, 64 },
      }
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 50, 65 }
      },
      build = {
         directory = dirname,
         basename = "build",
         frames = 4,
         columns = 2,
         rows = 2,
         hotspot = { 50, 61 }
      },
      working = {
         directory = dirname,
         basename = "working",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 53, 65 }
      }
   },

   aihints = {
      prohibited_till = 1800
   },

   working_positions = {
      europeans_miller_basic = 1
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
      },
      produce_mixed_flour = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding blackroot because ...
         descname = _"grinding rye and wheat",
         actions = {
            -- No check whether we need blackroot_flour because blackroots cannot be used for anything else.
            "return=skipped when site has corn and economy needs cornmeal and not economy needs flour",
            "consume=wheat:2 rye:2",
            "sleep=duration:45s",
            "playsound=sound/mill/mill_turning priority:85% allow_multiple",
            "animate=working duration:45s",
            "produce=flour:3"
         }
      },
   },
}

pop_textdomain()
