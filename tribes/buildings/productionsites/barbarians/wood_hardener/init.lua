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
         template = "idle_??",
         directory = dirname,
         hotspot = { 52, 64 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 52, 64 },
      },
      working = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 52, 64 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 52, 64 },
      },
   },

   aihints = {
      forced_after = 250,
      prohibited_till = 250
   },

   working_positions = {
      barbarians_lumberjack = 1
   },

   inputs = {
      log = 8
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
            "animate=working 24000",
            "produce=blackwood"
         }
      },
   },
}
