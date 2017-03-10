dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_donkeyfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Donkey Farm"),
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
         hotspot = { 85, 78 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 85, 78 },
      },
   },

   aihints = {
      recruitment = true
   },

   working_positions = {
      empire_donkeybreeder = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "wheat", amount = 8 }
   },
   outputs = {
      "empire_donkey"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing donkeys because ...
         descname = pgettext("empire_building", "rearing donkeys"),
         actions = {
            "sleep=15000",
            "return=skipped unless economy needs empire_donkey",
            "consume=wheat water",
            "play_sound=sound/farm donkey 192",
            "animate=working 15000", -- Feeding cute little baby donkeys ;)
            "recruit=empire_donkey"
         }
      },
   },
}
