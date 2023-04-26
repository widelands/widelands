push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Tavern"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "barbarians_inn",
      enhancement_cost = {
         log = 2,
         grout = 2,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         log = 1,
         grout = 1
      }
   },

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 1,
      reed = 1
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 57, 88 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 51, 70 }
      },
      working = {
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 51, 70 }
      },
   },

   aihints = {
      basic_amount = 1,
      prohibited_till = 520
   },

   working_positions = {
      barbarians_innkeeper = 1
   },

   inputs = {
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "barbarians_bread", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _("preparing a ration"),
         actions = {
            -- time total: 23.4 + 10 + 3.6 = 37 sec
            "return=skipped unless economy needs ration",
            "consume=barbarians_bread,fish,meat",
            "playsound=sound/barbarians/taverns/tavern priority:80%",
            "animate=working duration:23s400ms",
            "sleep=duration:10s",
            "produce=ration"
         },
      },
   },
}

pop_textdomain()
