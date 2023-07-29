push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_inn",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Inn"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "barbarians_big_inn",
      enhancement_cost = {
         log = 1,
         grout = 3,
         reed = 2
      },
      enhancement_return_on_dismantle = {
         grout = 2
      }
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
         hotspot = { 51, 81 }
      },
      working = {
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 51, 81 }
      },
   },

   aihints = {},

   working_positions = {
      barbarians_innkeeper = 1
   },

   inputs = {
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "barbarians_bread", amount = 4 },
      { name = "beer", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- time total: 37 + 41 = 78 sec
            "call=produce_ration",
            "call=produce_snack",
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _("preparing a ration"),
         actions = {
            -- time: 23.4 + 10 + 3.6 = 37 sec
            "return=skipped unless economy needs ration",
            "consume=barbarians_bread,fish,meat",
            "playsound=sound/barbarians/taverns/inn priority:80%",
            "animate=working duration:23s400ms",
            "sleep=duration:10s",
            "produce=ration"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _("preparing a snack"),
         actions = {
            -- time: 27.4 + 10 + 3.6 = 41 sec
            "return=skipped unless economy needs snack",
            "consume=barbarians_bread fish,meat beer",
            "playsound=sound/barbarians/taverns/inn priority:80%",
            "animate=working duration:27s400ms",
            "sleep=duration:10s",
            "produce=snack"
         }
      },
   },
}

pop_textdomain()
