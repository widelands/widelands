push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smelting_works_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Smelting Works"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 53 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 39, 53 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 39, 53 },
         fps = 5
      },
   },

   aihints = {
      prohibited_till = 7200
   },

   working_positions = {
      europeans_smelter_advanced = 1
   },

   inputs = {
      { name = "coal", amount = 16 },
      { name = "ore", amount = 8 },
      { name = "scrap_iron", amount = 8 },
      { name = "scrap_metal_mixed", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=recycle_iron",
            "call=recycle_gold",
            "call=smelt_iron",
            "call=smelt_gold",
         }
      },
      -- 2 identical programs for iron to prevent unnecessary skipping penalty
      recycle_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start recyling iron because ...
         descname = _"recycling iron",
         actions = {
            "return=skipped unless economy needs iron",
            "consume=scrap_iron:2 coal:2",
            "sleep=duration:15s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron:2"
         }
      },
      recycle_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start recyling gold because ...
         descname = _"recycling iron and gold",
         actions = {
            "return=skipped unless economy needs gold",
            "consume=scrap_metal_mixed:3 coal:3",
            "sleep=duration:15s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/goldping priority:60%",
            "produce=iron gold:2"
         }
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _"smelting iron",
         actions = {
            "return=skipped unless economy needs iron",
            "return=skipped when economy needs gold",
            "return=skipped when economy needs coal",
            "consume=ore:3 coal:3",
            "sleep=duration:15s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron:3",
         }
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
         descname = _"smelting gold",
         actions = {
            "return=skipped unless economy needs gold",
            "return=skipped when economy needs iron",
            "consume=ore:2 coal:2",
            "sleep=duration:15s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/goldping priority:60%",
            "produce=gold"
         }
      },
   },
}

pop_textdomain()
