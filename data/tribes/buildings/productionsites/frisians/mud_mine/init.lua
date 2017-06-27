dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_mud_mine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Mud Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      granite = 1,
      log = 3,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 88 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 88 },
      },
   },

   aihints = {
      forced_after = 900
   },

   working_positions = {
      frisians_digger = 1
   },

   inputs = {
      --{ name = "ration", amount = 6 },
   },
   outputs = {
      "mud"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"digging up earth",
         actions = {
            "sleep=10000",
            "return=skipped unless economy needs mud",
            --"consume=ration",
            "animate=working 24000",
            "produce=mud"
         },
      },
   },
}
