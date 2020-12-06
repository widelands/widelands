push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smelting_works_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Smelting Works"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_smelting_works_normal",
        enhancement_cost = {
          planks = 2,
          brick = 2,
          grout = 2,
          spidercloth = 2,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 2,
          quartz = 1,
        },
   },

   buildcost = {
      planks = 3,
      reed = 2,
      granite = 2
   },
   return_on_dismantle = {
      log = 2,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 44, 58 },
      },
   },

   aihints = {
      basic_amount = 1,
   },

   working_positions = {
      europeans_smelter_basic = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "ore", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smelt_iron_basic",
            "call=smelt_iron",
         }
      },
      smelt_iron_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _"smelting iron",
         actions = {
            "return=skipped unless economy needs iron",
            "consume=ore coal",
            "sleep=duration:25s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:25s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron",
         }
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _"smelting iron",
         actions = {
            "return=skipped unless economy needs iron",
            "return=skipped when economy needs coal",
            "consume=ore:3 coal:3",
            "sleep=duration:25s",
            "playsound=sound/metal/fizzle priority:20% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron:3",
         }
      },
   },
}

pop_textdomain()
