dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_horsefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Horse Farm"),
   helptext_script = dirname .. "helptexts.lua",
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 81, 62 },
      }
   },

   aihints = {
      recruitment = true
   },

   working_positions = {
      atlanteans_horsebreeder = 1
   },

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "corn", amount = 8 },
      { name = "water", amount = 8 }
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
            "play_sound=sound/farm horse 192",
            "animate=working 15000", -- Feeding cute little foals ;)
            "recruit=atlanteans_horse"
         }
      },

   },
}
