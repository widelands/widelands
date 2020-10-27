push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_donkeyfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Donkey Farm"),
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
      prohibited_till = 610,
   },

   working_positions = {
      empire_donkeybreeder = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "wheat", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing donkeys because ...
         descname = pgettext("empire_building", "rearing donkeys"),
         actions = {
            "return=skipped unless economy needs empire_donkey",
            "consume=wheat water",
            "sleep=duration:15s",
            "playsound=sound/farm/donkey priority:50% allow_multiple",
            "animate=working duration:15s", -- Feeding cute little baby donkeys ;)
            "recruit=empire_donkey"
         }
      },
   },
}

pop_textdomain()
