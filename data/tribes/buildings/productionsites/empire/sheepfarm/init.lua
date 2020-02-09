dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_sheepfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Sheep Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 73, 60 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 73, 60 },
      },
   },

   aihints = {
      prohibited_till = 380,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      empire_shepherd = 1
   },

   inputs = {
      { name = "water", amount = 7 },
      { name = "wheat", amount = 7 }
   },
   outputs = {
      "wool"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding sheep because ...
         descname = _"breeding sheep",
         actions = {
            "return=skipped unless economy needs wool",
            "consume=water wheat",
            "sleep=25000",
            "playsound=sound/farm/sheep 192",
            "animate=working 30000",
            "produce=wool"
         }
      },
   },
}
