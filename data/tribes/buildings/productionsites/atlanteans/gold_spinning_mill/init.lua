dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_gold_spinning_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Gold Spinning Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

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
         hotspot = { 33, 64 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 33, 64 },
      }
   },

   aihints = {
      prohibited_till = 600
   },

   working_positions = {
      atlanteans_carrier = 1
   },

   inputs = {
      { name = "gold", amount = 5 }
   },
   outputs = {
      "gold_thread"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start spinning gold because ...
         descname = _"spinning gold",
         actions = {
            "sleep=15000",
            "return=skipped unless economy needs gold_thread",
            "consume=gold",
            "play_sound=sound/atlanteans goldspin 192",
            "animate=working 25000",
            "produce=gold_thread"
         }
      },
   },
}
