dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_armorsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Armor Smithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 2,
      quartz = 1
   },
   return_on_dismantle = {
      granite = 1,
      planks = 1,
      quartz = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 60 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 53, 60 },
      }
   },

   aihints = {
      prohibited_till = 900
   },

   working_positions = {
      atlanteans_armorsmith = 1
   },

   inputs = {
      iron = 8,
      gold = 8,
      coal = 8
   },
   outputs = {
      "shield_advanced",
      "shield_steel"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_shield_steel",
            "call=produce_shield_advanced",
            "return=skipped"
         }
      },
      produce_shield_steel = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a steel shield because ...
         descname = _"forging a steel shield",
         actions = {
            "return=skipped unless economy needs shield_steel",
            "sleep=32000",
            "consume=iron:2 coal:2",
            "animate=working 35000",
            "produce=shield_steel"
         }
      },
      produce_shield_advanced = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an advanced shield because ...
         descname = _"forging an advanced shield",
         actions = {
            "return=skipped unless economy needs shield_advanced",
            "consume=iron:2 coal:2 gold",
            "sleep=32000",
            "animate=working 45000",
            "produce=shield_advanced"
         }
      },
   },
}
