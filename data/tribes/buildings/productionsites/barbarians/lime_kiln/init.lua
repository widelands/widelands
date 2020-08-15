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
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      barbarians_lime_burner = 1
   },

   inputs = {
      { name = "granite", amount = 6 },
      { name = "water", amount = 6 },
      { name = "coal", amount = 3 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mixing grout because ...
         descname = _"mixing grout",
         actions = {
            "return=skipped unless economy needs grout",
            "consume=coal granite:2 water:2",
            "sleep=duration:50s",
            "playsound=sound/barbarians/stonegrind priority:80%",
            "animate=working duration:29s",
            "playsound=sound/barbarians/mortar priority:60%",
            "sleep=duration:3s",
            "produce=grout:2"
         }
      },
   },
}
