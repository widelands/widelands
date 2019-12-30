dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_piggery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Piggery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      marble_column = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      marble = 1
   },


   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 82, 74 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 82, 74 },
      },
   },

   aihints = {
      prohibited_till = 590
   },

   working_positions = {
      empire_pigbreeder = 1
   },

   inputs = {
      { name = "water", amount = 7 },
      { name = "wheat", amount = 7 }
   },
   outputs = {
      "meat"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start raising pigs because ...
         descname = _"raising pigs",
         actions = {
            "return=skipped unless economy needs meat",
            "consume=water wheat",
            "sleep=10000",
            "playsound=sound/farm/farm_animal 180",
            "animate=working 20000",
            "produce=meat"
         }
      },
   },
}
