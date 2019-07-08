dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_wood_hardener",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Wood Hardener"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      granite = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 64 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 52, 64 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 52, 64 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 52, 64 },
      },
   },

   aihints = {
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      barbarians_lumberjack = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },
   outputs = {
      "blackwood"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start hardening wood because ...
         descname = _"hardening wood",
         actions = {
            "return=skipped unless economy needs blackwood",
            "consume=log:2",
            "sleep=43000",
            "playsound=sound/barbarians/blackwood 80",
            "animate=working 24000",
            "produce=blackwood"
         }
      },
   },
}
