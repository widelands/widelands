dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Tavern"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_inn",

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 1,
      thatch_reed = 1
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 57, 88 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 57, 88 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 57, 88 },
      },
   },

   aihints = {
      forced_after = 900
   },

   working_positions = {
      barbarians_innkeeper = 1
   },

   inputs = {
      fish = 4,
      barbarians_bread = 4,
      meat = 4
   },
   outputs = {
      "ration"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "sleep=14000",
            "return=skipped unless economy needs ration",
            "consume=barbarians_bread,fish,meat",
            "animate=working 19000",
            "produce=ration"
         },
      },
   },
}
