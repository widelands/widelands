dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_brick_burners_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Brick Burner´s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 3,
      granite = 4,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      granite = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 85 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 85 },
      },
   },

   aihints = {
      forced_after = 400
   },

   working_positions = {
      frisians_clay_burner = 1
   },

   inputs = {
      { name = "granite", amount = 6 },
      { name = "clay", amount = 6 },
      { name = "coal", amount = 4 },
   },
   outputs = {
      "brick"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start burning bricks because ...
         descname = _"burning bricks",
         actions = { --total time: 28s for 2 bricks
            "sleep=9000",
            "return=skipped unless economy needs brick",
            "consume=granite:2 clay:2 coal",
            "animate=working 17000",
            "produce=brick:2"
         },
      },
   },
}
