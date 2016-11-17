dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_smelting_works",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Smelting Works"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      granite = 4,
      planks = 1,
      spidercloth = 1
   },
   return_on_dismantle = {
      granite = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 72 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 57, 72 },
      }
   },

   aihints = {
      prohibited_till = 600,
      forced_after = 2400,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_smelter = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron_ore", amount = 8 },
      { name = "gold_ore", amount = 8 }
   },
   outputs = {
      "iron",
      "gold"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smelt_iron",
            "call=smelt_gold",
            "call=smelt_iron",
            "return=skipped"
         }
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _"smelting iron",
         actions = {
            "return=skipped unless economy needs iron",
            "sleep=5000",  -- penalty for unavailable iron_ore (2x)
            "consume=iron_ore coal",
            "sleep=20000",
            "play_sound=sound/metal fizzle 150",
            "animate=working 35000",
            "play_sound=sound/metal ironping 80",
            "produce=iron"
         }
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
         descname = _"smelting gold",
         actions = {
            "return=skipped unless economy needs gold",
            "sleep=10000",  -- penalty for unavailable gold_ore
            "consume=gold_ore coal",
            "sleep=15000",
            "play_sound=sound/metal fizzle 150",
            "animate=working 35000",
            "play_sound=sound/metal goldping 80",
            "produce=gold"
         }
      },
   },
}
