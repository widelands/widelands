push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Tavern"),
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_inn",

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
         descname = _"preparing a ration",
         actions = {
             -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=barbarians_bread,fish,meat",
            "playsound=sound/barbarians/taverns/tavern priority:80%",
            "animate=working duration:23s",
            "sleep=duration:10s",
            "produce=ration"
         },
      },
   },
}

pop_textdomain()
