push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smelting_works_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Smelting Works"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_smelting_works_advanced",
        enhancement_cost = {
          marble_column = 2,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 2,
          quartz = 1,
          diamond = 1
        },
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
      prohibited_till = 3600
   },

   working_positions = {
      europeans_smelter_advanced = 1
   },

   inputs = {
      { name = "coal", amount = 12 },
      { name = "ore", amount = 12 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smelt_iron",
            "call=smelt_gold",
            "call=smelt_iron_2",
         }
      },
      -- 2 identical programs for iron to prevent unnecessary skipping penalty
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _"smelting iron",
         actions = {
            "return=skipped unless economy needs iron",
            "consume=ore coal",
            "sleep=duration:20s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:25s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron",
            "return=skipped when economy needs coal",
            "consume=ore:3 coal:3",
            "sleep=duration:20s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron:3",
         }
      },
      smelt_iron_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _"smelting iron",
         actions = {
            "return=skipped unless economy needs iron",
            "return=skipped when economy needs coal",
            "consume=ore:2 coal:2",
            "sleep=duration:20s",
            "playsound=sound/metal/fizzle priority:15% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron:2"
         }
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
         descname = _"smelting gold",
         actions = {
            "return=skipped unless economy needs gold",
            "return=skipped when economy needs coal",
            "consume=ore:2 coal:2",
            "sleep=duration:20s",
            "playsound=sound/metal/fizzle priority:15% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/goldping priority:60%",
            "produce=gold"
         }
      },
   },
}

pop_textdomain()
