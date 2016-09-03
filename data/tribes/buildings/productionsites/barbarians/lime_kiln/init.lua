dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_lime_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Lime Kiln"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 4,
      granite = 2,
      blackwood = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 45, 53 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 45, 53 },
      },
   },

   aihints = {
      forced_after = 600,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      barbarians_lime_burner = 1
   },

   inputs = {
      { name = "granite", amount = 6 },
      { name = "water", amount = 6 },
      { name = "coal", amount = 3 }
   },
   outputs = {
      "grout"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mixing grout because ...
         descname = _"mixing grout",
         actions = {
            "sleep=50000",
            "return=skipped unless economy needs grout",
            "consume=coal granite:2 water:2",
            "play_sound=sound/barbarians stonegrind 100",
            "animate=working 29000",
            "play_sound=sound/barbarians mortar 80",
            "sleep=3000",
            "produce=grout:2"
         }
      },
   },
}
