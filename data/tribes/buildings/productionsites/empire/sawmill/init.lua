dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_sawmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Sawmill"),
   helptext_script = dirname .. "helptexts.lua",
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 54, 70 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 54, 70 },
      },
   },

   aihints = {
      basic_amount = 2,
      very_weak_ai_limit = 2,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_carpenter = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
         descname = _"sawing logs",
         actions = {
            "return=skipped unless economy needs planks",
            "consume=log:2",
            "sleep=16500", -- Much faster than barbarians' wood hardener
            "playsound=sound/sawmill/sawmill 180",
            "animate=working duration:20s", -- Much faster than barbarians' wood hardener
            "produce=planks"
         }
      },
   },
}
