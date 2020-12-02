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
   },

   working_positions = {
      europeans_miller_basic = 1
   },

   inputs = {
      { name = "corn", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"grinding corn",
         actions = {
            "return=skipped unless economy needs cornmeal",
            "return=skipped when economy needs corn",
            "sleep=duration:10s",
            "consume=corn:3",
            "playsound=sound/mill/mill_turning 240",
            "animate=working duration:30s",
            "produce=cornmeal:3"
         },
      },
   },
}

pop_textdomain()
