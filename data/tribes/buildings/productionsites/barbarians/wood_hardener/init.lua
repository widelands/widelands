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
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 52, 64 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 52, 64 },
      },
   },

   aihints = {
      is_basic = true,
      prohibited_till = 250,
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
            "sleep=43000",
            "return=skipped unless economy needs blackwood",
            "consume=log:2",
            "play_sound=sound/barbarians blackwood 80",
            "animate=working 24000",
            "produce=blackwood"
         }
      },
   },
}
