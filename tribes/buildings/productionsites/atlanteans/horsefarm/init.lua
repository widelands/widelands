dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_horsefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Horse Farm"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 81, 62 },
      },
      working = {
         template = "idle_??", -- TODO(GunChleoc): No animation yet.
         directory = dirname,
         hotspot = { 81, 62 },
      }
   },

   aihints = {
      recruitment = true
   },

   working_positions = {
      atlanteans_horsebreeder = 1
   },

   inputs = {
      corn = 8,
      water = 8
   },
   outputs = {
      "atlanteans_horse"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding horses because ...
         descname = _"breeding horses",
         actions = {
            "sleep=15000",
            "return=skipped unless economy needs atlanteans_horse",
            "consume=corn water",
            "playFX=sound/farm horse 192",
            "animate=working 15000", -- Feeding cute little foals ;)
            "recruit=atlanteans_horse"
         }
      },

   },
}