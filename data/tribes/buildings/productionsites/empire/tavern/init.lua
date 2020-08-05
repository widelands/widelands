dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Tavern"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "empire_inn",

   buildcost = {
      planks = 2,
      granite = 2,
      marble = 1
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1,
      marble = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 52, 58 },
      },
   },

   aihints = {
      basic_amount = 1,
      prohibited_till = 570
   },

   working_positions = {
      empire_innkeeper = 1
   },

   inputs = {
      { name = "fish", amount = 5 },
      { name = "meat", amount = 5 },
      { name = "empire_bread", amount = 5 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=empire_bread,fish,meat",
            "sleep=duration:5s",
            "playsound=sound/empire/taverns/ration 100",
            "animate=working duration:18s",
            "sleep=duration:10s",
            "produce=ration"
         }
      },
   },
}
