dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_helmsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Helm Smithy"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      blackwood = 1,
      granite = 2,
      grout = 2,
      thatch_reed = 3
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      grout = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 65, 83 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 65, 83 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 65, 83 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 65, 83 },
         fps = 20
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      barbarians_helmsmith = 1
   },

   inputs = {
      iron = 8,
      gold = 8,
      coal = 8
   },
   outputs = {
      "helmet",
      "helmet_mask",
      "helmet_warhelm"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_helmet",
            "call=produce_helmet_mask",
            "call=produce_helmet_warhelm",
            "return=skipped"
         }
      },
      produce_helmet = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _"forging a helmet",
         actions = {
            "return=skipped unless economy needs helmet",
            "sleep=32000",
            "consume=coal iron",
            "animate=working 35000",
            "produce=helmet"
         }
      },
      produce_helmet_mask = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a mask because ...
         descname = _"forging a mask",
         actions = {
            "return=skipped unless economy needs helmet_mask",
            "sleep=32000",
            "consume=coal iron:2",
            "animate=working 45000",
            "produce=helmet_mask"
         }
      },
      produce_helmet_warhelm = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a warhelm because ...
         descname = _"forging a warhelm",
         actions = {
            "return=skipped unless economy needs helmet_warhelm",
            "sleep=32000",
            "consume=coal gold iron:2",
            "animate=working 55000",
            "produce=helmet_warhelm"
         }
      },
   },
}