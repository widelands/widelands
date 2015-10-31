dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_sawmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Sawmill"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 1,
      planks = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 54, 70 },
      },
      working = {
         template = "idle_??", -- TODO(GunChleoc): No animation yet.
         directory = dirname,
         hotspot = { 54, 70 },
      },
   },

   aihints = {
      forced_after = 250,
      prohibited_till = 250
   },

   working_positions = {
      empire_carpenter = 1
   },

   inputs = {
      log = 8
   },
   outputs = {
      "planks"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
         descname = _"sawing logs",
         actions = {
            "sleep=16500", -- Much faster than barbarians' wood hardener
            "return=skipped unless economy needs planks",
            "consume=log:2",
            "playFX=sound/sawmill sawmill 180",
            "animate=working 20000", -- Much faster than barbarians' wood hardener
            "produce=planks"
         }
      },
   },
}