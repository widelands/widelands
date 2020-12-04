push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_sawmill_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Sawmill"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_sawmill_normal",
        enhancement_cost = {
          planks = 2,
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
      log = 3,
      reed = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 1
   },

   -- TODO(Nordfriese): Make animations
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 62, 48 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 62, 48 },
      }
   },

   aihints = {
        basic_amount = 1
   },

   working_positions = {
      europeans_carpenter_basic = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"sawing logs",
         actions = {
            "return=skipped unless economy needs planks",
            "return=skipped when economy needs log",
            "sleep=duration:20s",
            "return=skipped when not site has log:4",
            "consume=log:4",
            "playsound=sound/atlanteans/saw/benchsaw priority:50% allow_multiple",
            "animate=working duration:40s",
            "produce=planks:3"
         }
      },
   },
}

pop_textdomain()
