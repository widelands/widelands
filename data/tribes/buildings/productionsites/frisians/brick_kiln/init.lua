dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_brick_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Brick Kiln"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 83},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 86},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
   },

   aihints = {
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3,
   },

   working_positions = {
      frisians_brickmaker = 1
   },

   inputs = {
      { name = "granite", amount = 3 },
      { name = "clay", amount = 6 },
      { name = "coal", amount = 3 },
   },
   outputs = {
      "brick"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start burning bricks because ...
         descname = _"burning bricks",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs brick",
            "consume=granite clay:3 coal",
            "animate=working 50000",
            "produce=brick:3"
         },
      },
   },
}
